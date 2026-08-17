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
| `dataReturnOut` | output | Returns ownership of buffers back to the deframer, restoring the context saved at hand-off |
| `kemOut` | output | Forwards `KEM_ESTABLISHMENT` packets |
| `kemBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `kemOut` |
| `epOut` | output | Forwards `EP_PDU` packets |
| `epBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `epOut` |
| `passThroughOut` | output | Forwards all other packets, unchanged |
| `passThroughBufferReturnIn` | input, guarded | Receives back ownership of buffers sent on `passThroughOut` |

### 2.2 Buffer-context tracking

Each of the three output ports passes buffer ownership downstream; the corresponding `*BufferReturnIn` port carries no context, so the component records the outgoing `ComCfg::FrameContext` in a fixed-size table keyed by the buffer's data pointer (mirroring `Svc.FprimeRouter`'s pattern) and restores it on `dataReturnOut` when the buffer comes back. If the table is full, the buffer is still forwarded (a `BufferContextTableFull` event is raised) and its context degrades to empty on return.

## 3. Requirements

| ID | Description | Validation |
|---|---|---|
| KEYMGMTAPIDROUTER-001 | Packets with APID `KEM_ESTABLISHMENT` shall be forwarded on `kemOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-002 | Packets with APID `EP_PDU` shall be forwarded on `epOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-003 | Packets with any other APID shall be forwarded on `passThroughOut` only. | Unit Test |
| KEYMGMTAPIDROUTER-004 | The context associated with a buffer handed off on `kemOut`/`epOut`/`passThroughOut` shall be restored on `dataReturnOut` when the same buffer returns on the corresponding `*BufferReturnIn` port. | Unit Test |
| KEYMGMTAPIDROUTER-005 | A buffer returned without a matching table entry shall still be returned, with an empty context and a `BufferContextNotFound` event. | Unit Test |
