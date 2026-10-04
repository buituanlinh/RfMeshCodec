#include <Arduino.h>
#include "RfMeshCodec.h"

// ===== Helper: dump hex =====
static void dumpHex(const uint8_t* b, size_t n) {
  for (size_t i = 0; i < n; i++) {
    if (b[i] < 0x10) Serial.print('0');
    Serial.print(b[i], HEX);
    if (i + 1 < n) Serial.print(' ');
  }
  Serial.println();
}

static uint8_t crc8_msg_body(uint8_t msg_type, const void* body, size_t body_len) {
  const uint8_t* b = (const uint8_t*)body;
  uint8_t crc = 0x00;
  crc = rf_crc8_maxim_update(crc, msg_type);
  for (size_t i = 0; i < body_len; i++) crc = rf_crc8_maxim_update(crc, b[i]);
  return crc;
}

static void print_test_vector(const __FlashStringHelper* name,
                              uint8_t msg_type,
                              const void* body, size_t body_len,
                              const uint8_t* payload, size_t payload_len) {
  Serial.print(F("[VEC] "));
  Serial.print(name);
  Serial.print(F("  msg_type='"));
  Serial.print((char)msg_type);
  Serial.print(F("'  body_len="));
  Serial.print((unsigned)body_len);
  Serial.print(F("  payload_len="));
  Serial.println((unsigned)payload_len);

  const uint8_t crc = crc8_msg_body(msg_type, body, body_len);
  Serial.print(F("      crc8(msg_type||body)=0x"));
  if (crc < 0x10) Serial.print('0');
  Serial.println(crc, HEX);

  Serial.print(F("      payload: "));
  dumpHex(payload, payload_len);
}

// ===== Test: encode/decode G (cfg_req) =====
static bool test_cfg_req_G() {
  rf_body_G_t g;
  g.h.ver = RF_CODEC_PROTO_VER;
  g.h.flags = 0;
  g.req_id_le = rf_le16(0x1234);
  g.uptime_ms_le = rf_le32(0x01020304UL);

  uint8_t payload[32];
  size_t out_len = 0;

  const bool ok_enc = rf_encode_payload(RF_MSG_G_CFG_REQ, &g, sizeof(g), payload, sizeof(payload), &out_len);
  if (!ok_enc) {
    Serial.println(F("[FAIL] encode G"));
    return false;
  }

  Serial.print(F("[OK] encode G len="));
  Serial.println((unsigned)out_len);
  Serial.print(F("payload: "));
  dumpHex(payload, out_len);

  print_test_vector(F("G_cfg_req"), RF_MSG_G_CFG_REQ, &g, sizeof(g), payload, out_len);

  // Decode
  const uint8_t* body = nullptr;
  size_t body_len = 0;
  rf_decode_status_t st = rf_decode_payload(RF_MSG_G_CFG_REQ, payload, out_len, &body, &body_len);
  if (st != RF_DECODE_OK) {
    Serial.print(F("[FAIL] decode G st="));
    Serial.println((unsigned)st);
    return false;
  }

  if (body_len != sizeof(rf_body_G_t)) {
    Serial.println(F("[FAIL] decode G body_len mismatch"));
    return false;
  }

  const rf_body_G_t* gg = (const rf_body_G_t*)body;
  const uint16_t req_id = rf_le16(gg->req_id_le);
  const uint32_t up_ms  = rf_le32(gg->uptime_ms_le);

  if (req_id != 0x1234 || up_ms != 0x01020304UL) {
    Serial.println(F("[FAIL] decode G fields mismatch"));
    return false;
  }

  Serial.println(F("[OK] decode G fields match"));
  return true;
}

// ===== Test: encode/decode C (cmd) =====
static bool test_cmd_C() {
  uint8_t payload[32];
  size_t out_len = 0;

  const bool ok = rf_build_cmd_C(
    0xBEEF,      // req_id
    0x0011,      // issued_ms16
    1500,        // ttl_ms
    7,           // target_id
    RF_CMD_SET,  // mode
    (int16_t)123,// value
    0x2222,      // param
    payload, sizeof(payload), &out_len
  );

  if (!ok) {
    Serial.println(F("[FAIL] build C"));
    return false;
  }

  Serial.print(F("[OK] encode C len="));
  Serial.println((unsigned)out_len);
  Serial.print(F("payload: "));
  dumpHex(payload, out_len);

  // Decode 1 lần để lấy body chuẩn (phục vụ in test vector)
  const uint8_t* body_vec = nullptr;
  size_t body_vec_len = 0;
  rf_decode_status_t stv = rf_decode_payload(RF_MSG_C_CMD, payload, out_len, &body_vec, &body_vec_len);
  if (stv == RF_DECODE_OK) {
    print_test_vector(F("C_cmd"), RF_MSG_C_CMD, body_vec, body_vec_len, payload, out_len);
  } else {
    Serial.println(F("[WARN] cannot build C test vector (decode failed)"));
  }

  // Decode
  const uint8_t* body = nullptr;
  size_t body_len = 0;
  rf_decode_status_t st = rf_decode_payload(RF_MSG_C_CMD, payload, out_len, &body, &body_len);
  if (st != RF_DECODE_OK) {
    Serial.print(F("[FAIL] decode C st="));
    Serial.println((unsigned)st);
    return false;
  }

  if (body_len != sizeof(rf_body_C_t)) {
    Serial.println(F("[FAIL] decode C body_len mismatch"));
    return false;
  }

  const rf_body_C_t* cc = (const rf_body_C_t*)body;

  const uint16_t req_id      = rf_le16(cc->req_id_le);
  const uint16_t issued_ms16 = rf_le16(cc->issued_ms16_le);
  const uint16_t ttl_ms      = rf_le16(cc->ttl_ms_le);
  const uint16_t param       = rf_le16(cc->param_le);
  const int16_t  val         = (int16_t)rf_le16((uint16_t)cc->value_le);

  if (req_id != 0xBEEF || issued_ms16 != 0x0011 || ttl_ms != 1500 ||
      cc->target_id != 7 || cc->mode != RF_CMD_SET || val != 123 || param != 0x2222) {
    Serial.println(F("[FAIL] decode C fields mismatch"));
    return false;
  }

  Serial.println(F("[OK] decode C fields match"));
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println(F("=== RfMeshCodec codec_smoke ==="));

  // Compile-time sanity (in ra size để debug nhanh)
  Serial.print(F("sizeof(rf_body_g_t)="));
  Serial.println((unsigned)sizeof(rf_body_g_t));
  Serial.print(F("sizeof(rf_body_G_t)="));
  Serial.println((unsigned)sizeof(rf_body_G_t));
  Serial.print(F("sizeof(rf_body_C_t)="));
  Serial.println((unsigned)sizeof(rf_body_C_t));

  bool ok1 = test_cfg_req_G();
  bool ok2 = test_cmd_C();

  Serial.println(ok1 && ok2 ? F("=== ALL TESTS PASS ===") : F("=== TEST FAILED ==="));
}

void loop() {
  // no-op
}
