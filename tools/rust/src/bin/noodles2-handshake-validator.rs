use std::env;
use std::fmt;
use std::io::{Cursor, Write};
use std::net::{TcpListener, TcpStream};
use std::process::ExitCode;
use std::time::Duration;

use tungstenite::protocol::{Message, WebSocket, WebSocketConfig};

#[allow(warnings)]
mod wire {
    include!(concat!(env!("OUT_DIR"), "/noodles2.rs"));
}

use wire::{JawRead, JawWrite};

const DEFAULT_LISTEN: &str = "127.0.0.1:50000";
const DEFAULT_TIMEOUT_SECS: u64 = 10;
const DEFAULT_MAX_MESSAGE_BYTES: usize = 1024 * 1024;
const SERVER_NAME: &[u8] = b"NOODLES2 handshake validator";
const CORE_EXTENSION_ID: u16 = 0;
const AUDIO_EXTENSION_ID: u16 = 1;

fn main() -> ExitCode {
    match Config::parse(env::args().skip(1)) {
        Ok(ParseResult::Help) => {
            print_help();
            ExitCode::SUCCESS
        }
        Ok(ParseResult::Run(config)) => match run(config) {
            Ok(()) => ExitCode::SUCCESS,
            Err(error) => {
                eprintln!("FAIL: {error}");
                ExitCode::FAILURE
            }
        },
        Err(error) => {
            eprintln!("error: {error}\n");
            print_help();
            ExitCode::from(2)
        }
    }
}

fn run(config: Config) -> Result<(), ToolError> {
    let listener = TcpListener::bind(&config.listen)
        .map_err(|error| ToolError::new(format!("cannot listen on {}: {error}", config.listen)))?;
    let local_addr = listener
        .local_addr()
        .map_err(|error| ToolError::new(format!("cannot inspect listener address: {error}")))?;

    println!("LISTENING ws://{local_addr}");
    std::io::stdout()
        .flush()
        .map_err(|error| ToolError::new(format!("cannot flush status output: {error}")))?;

    let (stream, peer_addr) = listener
        .accept()
        .map_err(|error| ToolError::new(format!("accept failed: {error}")))?;
    configure_stream(&stream, config.timeout)?;

    let websocket_config = WebSocketConfig::default()
        .max_message_size(Some(config.max_message_bytes))
        .max_frame_size(Some(config.max_message_bytes));
    let mut socket =
        tungstenite::accept_with_config(stream, Some(websocket_config)).map_err(|error| {
            ToolError::new(format!(
                "WebSocket handshake from {peer_addr} failed: {error}"
            ))
        })?;

    let result = validate_connection(&mut socket, config.max_message_bytes);
    let _ = socket.close(None);
    result?;

    println!("PASS: valid introduction from {peer_addr}; IntroductionReply sent");
    Ok(())
}

fn configure_stream(stream: &TcpStream, timeout: Duration) -> Result<(), ToolError> {
    stream
        .set_read_timeout(Some(timeout))
        .map_err(|error| ToolError::new(format!("cannot set read timeout: {error}")))?;
    stream
        .set_write_timeout(Some(timeout))
        .map_err(|error| ToolError::new(format!("cannot set write timeout: {error}")))?;
    Ok(())
}

fn validate_connection(
    socket: &mut WebSocket<TcpStream>,
    max_message_bytes: usize,
) -> Result<(), ToolError> {
    let payload = read_first_binary_message(socket, max_message_bytes)?;
    let introduction = decode_introduction(&payload)?;
    validate_introduction(&introduction)?;

    let reply = make_reply();
    let mut encoded = Vec::new();
    reply
        .jaw_write(&mut encoded)
        .map_err(|error| ToolError::new(format!("cannot encode IntroductionReply: {error}")))?;
    socket
        .send(Message::Binary(encoded.into()))
        .map_err(|error| ToolError::new(format!("cannot send IntroductionReply: {error}")))?;
    Ok(())
}

fn read_first_binary_message(
    socket: &mut WebSocket<TcpStream>,
    max_message_bytes: usize,
) -> Result<Vec<u8>, ToolError> {
    loop {
        let message = socket
            .read()
            .map_err(|error| ToolError::new(format!("cannot read preamble message: {error}")))?;
        match message {
            Message::Binary(payload) => {
                if payload.is_empty() {
                    return Err(ToolError::new("empty binary preamble message"));
                }
                if payload.len() > max_message_bytes {
                    return Err(ToolError::new(format!(
                        "preamble message is {} bytes; limit is {max_message_bytes}",
                        payload.len()
                    )));
                }
                return Ok(payload.to_vec());
            }
            Message::Text(_) => {
                return Err(ToolError::new(
                    "received a WebSocket text message; NOODLES2 messages must be binary",
                ));
            }
            Message::Close(_) => {
                return Err(ToolError::new(
                    "client closed before sending an introduction",
                ));
            }
            Message::Ping(_) | Message::Pong(_) | Message::Frame(_) => {}
        }
    }
}

fn decode_introduction(payload: &[u8]) -> Result<wire::Introduction, ToolError> {
    let mut cursor = Cursor::new(payload);
    let message = wire::PreMessage::jaw_read(&mut cursor)
        .map_err(|error| ToolError::new(format!("malformed PreMessage: {error}")))?;
    if cursor.position() != payload.len() as u64 {
        return Err(ToolError::new(format!(
            "PreMessage has {} trailing byte(s)",
            payload.len() as u64 - cursor.position()
        )));
    }

    match message {
        wire::PreMessage::Introduction(introduction) => Ok(introduction),
        wire::PreMessage::Probe(_) => Err(ToolError::new(
            "received Probe; this tool validates Introduction handshakes only",
        )),
        wire::PreMessage::ProbeReply(_) => Err(ToolError::new(
            "received ProbeReply from a client; expected Introduction",
        )),
        wire::PreMessage::IntroductionReply(_) => Err(ToolError::new(
            "received IntroductionReply from a client; expected Introduction",
        )),
    }
}

fn validate_introduction(introduction: &wire::Introduction) -> Result<(), ToolError> {
    let header = &introduction.header;
    if header.protocol_version != wire::PROTOCOL_VERSION {
        return Err(ToolError::new(format!(
            "protocol version {} does not match {}",
            header.protocol_version,
            wire::PROTOCOL_VERSION
        )));
    }
    if header.flags.authority_request() != 0 {
        return Err(ToolError::new(
            "authority_request is set; this validator only acts as Upstream",
        ));
    }

    let capabilities: Vec<u16> = header.capabilities.0.iter().map(extension_id).collect();
    if !capabilities.contains(&CORE_EXTENSION_ID) {
        if capabilities.contains(&AUDIO_EXTENSION_ID) {
            return Err(ToolError::new(
                "Audio extension 0x0001 is missing its Core dependency 0x0000",
            ));
        }
        return Err(ToolError::new(
            "capability list does not advertise required Core extension 0x0000",
        ));
    }
    Ok(())
}

fn extension_id(extension: &wire::ExtensionID) -> u16 {
    unsafe { (&raw const extension.id).read_unaligned() }
}

fn make_reply() -> wire::PreMessage {
    wire::PreMessage::IntroductionReply(wire::IntroductionReply {
        header: wire::IntroductionHeader {
            protocol_version: wire::PROTOCOL_VERSION,
            flags: wire::IntroductionFlags::default(),
            name: wire::PreambleName(SERVER_NAME.to_vec()),
            capabilities: wire::PreambleCaps(vec![wire::ExtensionID {
                id: CORE_EXTENSION_ID,
            }]),
        },
        large_assets: wire::OOBAssetInfo::Unavailable,
        allocation_namespace: 1,
    })
}

#[derive(Debug)]
struct ToolError(String);

impl ToolError {
    fn new(message: impl Into<String>) -> Self {
        Self(message.into())
    }
}

impl fmt::Display for ToolError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        formatter.write_str(&self.0)
    }
}

#[derive(Debug)]
struct Config {
    listen: String,
    timeout: Duration,
    max_message_bytes: usize,
}

enum ParseResult {
    Help,
    Run(Config),
}

impl Config {
    fn parse(arguments: impl Iterator<Item = String>) -> Result<ParseResult, String> {
        let mut config = Self {
            listen: DEFAULT_LISTEN.to_owned(),
            timeout: Duration::from_secs(DEFAULT_TIMEOUT_SECS),
            max_message_bytes: DEFAULT_MAX_MESSAGE_BYTES,
        };
        let mut arguments = arguments.peekable();

        while let Some(argument) = arguments.next() {
            match argument.as_str() {
                "-h" | "--help" => return Ok(ParseResult::Help),
                "--listen" => {
                    config.listen = next_value(&mut arguments, "--listen")?;
                }
                "--timeout-secs" => {
                    let value = next_value(&mut arguments, "--timeout-secs")?;
                    let seconds = value
                        .parse::<u64>()
                        .map_err(|_| format!("invalid --timeout-secs value: {value}"))?;
                    if seconds == 0 {
                        return Err("--timeout-secs must be greater than zero".to_owned());
                    }
                    config.timeout = Duration::from_secs(seconds);
                }
                "--max-message-bytes" => {
                    let value = next_value(&mut arguments, "--max-message-bytes")?;
                    config.max_message_bytes = value
                        .parse::<usize>()
                        .map_err(|_| format!("invalid --max-message-bytes value: {value}"))?;
                    if config.max_message_bytes == 0 {
                        return Err("--max-message-bytes must be greater than zero".to_owned());
                    }
                }
                _ => return Err(format!("unknown argument: {argument}")),
            }
        }
        Ok(ParseResult::Run(config))
    }
}

fn next_value(
    arguments: &mut impl Iterator<Item = String>,
    option: &str,
) -> Result<String, String> {
    arguments
        .next()
        .ok_or_else(|| format!("missing value for {option}"))
}

fn print_help() {
    println!(
        "NOODLES2 one-shot WebSocket handshake validator\n\n\
         Usage: noodles2-handshake-validator [OPTIONS]\n\n\
         Options:\n\
           --listen ADDRESS           Listen address [default: {DEFAULT_LISTEN}]\n\
           --timeout-secs SECONDS     I/O timeout [default: {DEFAULT_TIMEOUT_SECS}]\n\
           --max-message-bytes BYTES  Preamble size limit [default: {DEFAULT_MAX_MESSAGE_BYTES}]\n\
           -h, --help                 Show this help"
    );
}

#[cfg(test)]
mod tests {
    use super::*;

    fn introduction(version: u16, capabilities: &[u16]) -> wire::Introduction {
        wire::Introduction {
            header: wire::IntroductionHeader {
                protocol_version: version,
                flags: wire::IntroductionFlags::default(),
                name: wire::PreambleName(b"test client".to_vec()),
                capabilities: wire::PreambleCaps(
                    capabilities
                        .iter()
                        .map(|id| wire::ExtensionID { id: *id })
                        .collect(),
                ),
            },
        }
    }

    #[test]
    fn accepts_core_and_duplicate_capabilities() {
        assert!(validate_introduction(&introduction(wire::PROTOCOL_VERSION, &[0, 0, 1])).is_ok());
    }

    #[test]
    fn rejects_missing_core() {
        let error = validate_introduction(&introduction(wire::PROTOCOL_VERSION, &[1]))
            .unwrap_err()
            .to_string();
        assert!(error.contains("Core"));
    }

    #[test]
    fn rejects_wrong_protocol_version() {
        assert!(validate_introduction(&introduction(wire::PROTOCOL_VERSION + 1, &[0])).is_err());
    }

    #[test]
    fn decoder_requires_introduction_and_exact_consumption() {
        let mut encoded = Vec::new();
        wire::PreMessage::Introduction(introduction(wire::PROTOCOL_VERSION, &[0]))
            .jaw_write(&mut encoded)
            .unwrap();
        assert!(decode_introduction(&encoded).is_ok());

        encoded.push(0xff);
        assert!(
            decode_introduction(&encoded)
                .unwrap_err()
                .to_string()
                .contains("trailing")
        );
    }

    #[test]
    fn reply_is_core_only_and_assigns_a_namespace() {
        let wire::PreMessage::IntroductionReply(reply) = make_reply() else {
            panic!("wrong reply variant");
        };
        assert_eq!(reply.header.protocol_version, wire::PROTOCOL_VERSION);
        assert_eq!(reply.header.capabilities.0.len(), 1);
        assert_eq!(
            extension_id(&reply.header.capabilities.0[0]),
            CORE_EXTENSION_ID
        );
        assert!(matches!(
            reply.large_assets,
            wire::OOBAssetInfo::Unavailable
        ));
        assert_ne!(reply.allocation_namespace, 0);
    }

    #[test]
    fn completes_a_real_websocket_handshake() {
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let address = listener.local_addr().unwrap();
        let server = std::thread::spawn(move || {
            let (stream, _) = listener.accept().unwrap();
            let mut socket = tungstenite::accept(stream).unwrap();
            validate_connection(&mut socket, DEFAULT_MAX_MESSAGE_BYTES).unwrap();
        });

        let (mut client, _) = tungstenite::connect(format!("ws://{address}")).unwrap();
        let mut encoded = Vec::new();
        wire::PreMessage::Introduction(introduction(wire::PROTOCOL_VERSION, &[0]))
            .jaw_write(&mut encoded)
            .unwrap();
        client.send(Message::Binary(encoded.into())).unwrap();

        let Message::Binary(reply) = client.read().unwrap() else {
            panic!("server did not return a binary IntroductionReply");
        };
        let mut cursor = Cursor::new(reply.as_ref());
        let decoded = wire::PreMessage::jaw_read(&mut cursor).unwrap();
        assert_eq!(cursor.position(), reply.len() as u64);
        assert!(matches!(decoded, wire::PreMessage::IntroductionReply(_)));
        server.join().unwrap();
    }
}
