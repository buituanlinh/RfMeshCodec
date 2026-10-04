# RfMeshCodec

**Step 1 (skeleton)**

- Transport-agnostic library intended to provide *binary pack/unpack* for AVR ↔ ESP32 frames.
- No `mesh.*`, `network.*`, retry/FSM, or gateway procedure logic.

Next steps will copy core protocol primitives from `RfMeshProto` and add type-safe `rf_pack<'X'>` / `rf_unpack<'X'>` APIs.
