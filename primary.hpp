#pragma once

#include "common.hpp"
#include "content.hpp"

enum class MessageID : u8 {
  TRANSACTION = 1,
  NAMED_TRANSACTION = 2,
  TRANSACTION_REPLY = 3,

  PROBE = 4,
  PROBE_REPLY = 5,

  INTRO = 6,
  INTRO_REPLY = 7,

  RPC = 8,

  RPC_INVOKE = 9,
  RPC_INVOKE_REPLY = 10,

  RPC_INVOKE_BATCH = 11,
  RPC_INVOKE_BATCH_REPLY = 12,
};

// =============================================

/// @brief Sent by connecting node, sent on first connection
struct Introduction {
  u16 protocol_version;
  bool authority_request;
  u8 padding;
  FixedShortString<64> name;
};

/// @brief Sent by remote node. Intro reply sent on successful introduction.
struct IntroductionReply {
  u16 protocol_version;
  u16 padding;

  FixedShortString<64> session_name;

  // Optional asset stream context
  // The protocol to use to obtain content. For now, MUST BE "ws" or "wss" for
  // websocket
  FixedShortString<16> large_asset_protocol;

  // DYN

  // The ports that you can connect to. The host is the same as the initial
  // connection.
  DynamicArray<u16, u16> large_asset_ports;
};

// =============================================

/// @brief Ask a node to identify itself. This message is intended to be used
/// in exclusion to all others. If sent, the server will send a query reply
/// and then MUST close the connection.
struct Probe {};

/// @brief Reply to a probe request.
/// For compatibility, the icon or representative image should be either in
/// WEPB, PNG, JPG, or SVG format. Note that while the spec has a u32 here for
/// the icon size, large content (2^24, ~16 mb) can be discarded or ignored.
struct ProbeReply {
  FixedShortString<64> session_name;

  // DYN

  DynamicByteArray<u32> icon;
};

// ==============================================================================
// Transaction message

using TransactionIdentifier = u32;

// Transactions have an identifier, to be used with replies.
// If the message is Upstream to Downstream, no ID is used
// - It is expected that the client will do as requested, and the server would
// not care if a transaction
//   was rejected.
// If the message is Downstream to Upstream, an ID _should_ be provided
// - In this case, the Named version is used.
// - The client will need an indication of a transaction accept or reject
// - The client may elect to omit the ID and thus signal it is not interested in
// a response
struct Transaction {
  // DYN

  DynamicArray<u32, ContentMessage> content;
};

struct NamedTransaction {
  TransactionIdentifier transaction_identifier;

  // DYN

  DynamicArray<u32, ContentMessage> content;
};

// Indication if the transaction was accepted or rejected.
class TransactionReply {
  TransactionIdentifier transaction_identifier;
  u32 code;
};