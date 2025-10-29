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

/// Sent by connecting node on first connection (Fixed-only)
/// Fixed: protocol fields and a short name; no Dyn part today.
struct IntroductionFixed {
  u16 protocol_version;
  // Flags: bit 0 = authority_request
  u16 flags;
  FixedShortString<64> name;
};
using Introduction = IntroductionFixed;

/// @brief Sent by remote node. Intro reply sent on successful introduction.
/// Introduction reply: Fixed header and Dyn ports
/// Fixed: negotiated protocol, flags, session name, and asset protocol hint.
struct IntroductionReplyFixed {
  u16 protocol_version;
  // Flags reserved for future use
  u16 flags;

  FixedShortString<64> session_name;

  // Optional asset stream context
  // The protocol to use to obtain content. For now, MUST BE "ws" or "wss" for
  // websocket
  FixedShortString<16> large_asset_protocol;
};

/// Dyn: port list for large-asset channel
struct IntroductionReplyDyn {
  // The ports that you can connect to. The host is the same as the initial
  // connection.
  DynamicArray<u16, u16> large_asset_ports;
};

// =============================================

/// @brief Ask a node to identify itself. This message is intended to be used
/// in exclusion to all others. If sent, the server will send a query reply
/// and then MUST close the connection.
/// Probe message (Fixed-only, empty marker)
struct ProbeFixed {};
using Probe = ProbeFixed;

/// @brief Reply to a probe request.
/// For compatibility, the icon or representative image should be either in
/// WEPB, PNG, JPG, or SVG format. Note that while the spec has a u32 here for
/// the icon size, large content (2^24, ~16 mb) can be discarded or ignored.
/// Probe reply: Fixed header only (session name)
struct ProbeReplyFixed {
  FixedShortString<64> session_name;
};

/// Probe reply: Dyn icon bytes
struct ProbeReplyDyn {
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
/// Transaction (Dyn-only): sequence of content messages
struct TransactionDyn {
  DynamicArray<u32, ContentMessage> content;
};
using Transaction = TransactionDyn;

/// Named transaction: Fixed identifier then Dyn content
struct NamedTransactionFixed {
  TransactionIdentifier transaction_identifier;
};

struct NamedTransactionDyn {
  DynamicArray<u32, ContentMessage> content;
};

// Indication if the transaction was accepted or rejected.
/// Transaction reply (Fixed-only): id + result code
struct TransactionReplyFixed {
  TransactionIdentifier transaction_identifier;
  u32 code;
};
using TransactionReply = TransactionReplyFixed;

// Fixed layout checks
N2_STATIC_ASSERT_FIXED(IntroductionFixed);
N2_STATIC_ASSERT_FIXED(IntroductionReplyFixed);
N2_STATIC_ASSERT_FIXED(ProbeFixed);
N2_STATIC_ASSERT_FIXED(ProbeReplyFixed);
N2_STATIC_ASSERT_FIXED(NamedTransactionFixed);
N2_STATIC_ASSERT_FIXED(TransactionReplyFixed);