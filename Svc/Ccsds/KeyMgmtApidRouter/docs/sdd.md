# Svc.Ccsds.KeyMgmtApidRouter

## 1. Introduction

`KeyMgmtApidRouter` splits SDLS key-management traffic off the deframed space-packet stream before it reaches the standard `Svc.Router`. Two APIDs receive special handling:

- `ComCfg.Apid.KEM_ESTABLISHMENT` — ML-KEM-768 ciphertext fragments, forwarded on `kemOut`
- `ComCfg.Apid.EP_PDU` — CCSDS 355.1-B-1 Extended Procedures PDUs (OTAR, Key Activation, Key Verification), forwarded on `epOut`

Everything else is forwarded unchanged on `passThroughOut`, normally connected to a standard router (e.g. `Svc.FprimeRouter`). `Svc.Router` implementations demux by `Fw::ComPacketType` (`ComCfg::Apid::T` cast from the closed F Prime packet-type set) and have no notion of these two APIDs, so `KeyMgmtApidRouter` sits upstream of the router and intercepts them by raw APID before the router ever sees them.

## 2. Design

### 2.1 Ports

| Port | Direction | Description |
|---|---|---|
| `dataIn` | input, guarded | Receives deframed space packets from a packet deframer (e.g. `Svc.Ccsds.SpacePacketDeframer`) |
| `dataReturnOut` | output | Returns ownership of buffers back to the deframer, with the context they came back with |
| `kemOut` | output | Forwards `KEM_ESTABLISHMENT` packets. Optional — see §2.3 |
| `kemBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `kemOut` |
| `epOut` | output | Forwards `EP_PDU` packets. Optional — see §2.3 |
| `epBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `epOut` |
| `passThroughOut` | output | Forwards all other packets, unchanged. Required — see §2.3 |
| `passThroughBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `passThroughOut` |

All eight ports are `Svc.ComDataWithContext`, so the `ComCfg::FrameContext` travels with the buffer in both directions.

### 2.2 Buffer ownership

Each of the three output ports passes buffer ownership downstream, and the corresponding `*BufferReturnIn` port carries the buffer *and* its context back. The component therefore holds no state: a return handler forwards straight to `dataReturnOut`. This is a deliberate difference from `Svc.FprimeRouter`, which keeps a buffer-to-context table because the `Svc.Router` interface it imports offers only a single buffer-only return port (`fileBufferReturnIn`) shared by `fileOut` and `unknownDataOut`. `KeyMgmtApidRouter` declares its own ports and so has no such constraint; carrying the context on the return ports removes the table, its fixed capacity, and the context-loss modes that come with them.

A consequence worth stating for downstream components: because there is no table keyed on buffer identity, a downstream handler is free to advance or re-slice the buffer before returning it, so long as it returns the context it was given.

### 2.3 Unconnected output ports

`kemOut` and `epOut` are optional. During bring-up — before the KEM reassembler and EP PDU handler exist — a key-management packet arriving with no handler connected would otherwise assert on an unconnected output port. Instead the component raises `KeyMgmtPortNotConnected` and returns the buffer on `dataReturnOut`.

`passThroughOut` carries all normal uplink traffic and is **not** guarded: as with `Svc.FprimeRouter`'s `commandOut`, leaving it unconnected is a topology error and should fail loudly.

## 3. Requirements

| ID | Description | Validation |
|---|---|---|
| KEYMGMTAPIDROUTER-001 | Packets with APID `KEM_ESTABLISHMENT` shall be forwarded on `kemOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-002 | Packets with APID `EP_PDU` shall be forwarded on `epOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-003 | Packets with any other APID shall be forwarded on `passThroughOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-004 | A packet forwarded on any output port shall carry the context it was received with. | Unit Test |
| KEYMGMTAPIDROUTER-005 | A buffer returned on any `*BufferReturnIn` port shall be forwarded on `dataReturnOut` with the context it was returned with, independent of how many buffers are outstanding or the order in which they return. | Unit Test |
| KEYMGMTAPIDROUTER-006 | A key-management packet whose output port is unconnected shall be returned on `dataReturnOut` and shall raise `KeyMgmtPortNotConnected`. | Inspection |
