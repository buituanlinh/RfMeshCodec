#include "rf_codec.h"
#include <string.h>
#include <stdio.h>

// ============================================================
// CRC-8/MAXIM
// ============================================================
uint8_t rf_crc8_maxim(const uint8_t* data, size_t len) {
  // CRC-8/MAXIM: init 0x00, xorout 0x00, refin/refout true
  // poly 0x31 => reversed poly 0x8C
  uint8_t crc = 0x00;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t b = 0; b < 8; b++) {
      crc = (crc & 0x01) ? (uint8_t)((crc >> 1) ^ 0x8C) : (uint8_t)(crc >> 1);
    }
  }
  return crc;
}

// ============================================================
// Node helper: send 'P'
// ============================================================
bool rf_node_send_P_once(
    rf_send_fn_t send_fn, void* user,
    uint16_t req_id,
    const uint8_t mac6[6],
    uint16_t model_id) {

  if (!send_fn || !mac6) return false;

  rf_body_P_t b;
  memset(&b, 0, sizeof(b));
  b.h.ver      = RF_PROTO_VER;
  b.h.flags    = 0;
  b.req_id_le  = req_id;          // firmware cấp
  memcpy(b.mac, mac6, 6);
  b.model_id_le = model_id;       // AVR/ESP32 đều little-endian

  uint8_t payload[sizeof(rf_body_P_t) + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u)];
  size_t out_len = 0;
  if (!rf_encode_payload((uint8_t)RF_MSG_P_PROVISION_REQ,
                         &b, sizeof(b),
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }
  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_P_PROVISION_REQ, payload, (uint8_t)out_len, user);
}

// ============================================================
// Node helper: send 'W'
// ============================================================
bool rf_node_send_W(
    rf_send_fn_t send_fn, void* user,
    uint16_t req_id,
    const uint8_t mac6[6],
    uint32_t uptime_s) {

  if (!send_fn || !mac6) return false;

  rf_body_W_t b;
  memset(&b, 0, sizeof(b));
  b.h.ver   = RF_PROTO_VER;
  b.h.flags = 0;
  b.req_id_le = req_id;
  memcpy(b.mac, mac6, 6);
  b.uptime_s_le = uptime_s;

  uint8_t payload[sizeof(rf_body_W_t) + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u)];
  size_t out_len = 0;

  if (!rf_encode_payload((uint8_t)RF_MSG_W_HWCFG_REQ,
                         &b, sizeof(b),
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }
  return send_fn((uint8_t)RF_MSG_W_HWCFG_REQ, payload, (uint8_t)out_len, user);
}

// ============================================================
// Node helper: send 'G'
// ============================================================
bool rf_node_send_G(
    rf_send_fn_t send_fn,
    void* user,
    uint16_t req_id,
    const uint8_t mac6[6],
    uint32_t uptime_s) {

  if (!send_fn || !mac6) return false;

  rf_body_G_t b;
  memset(&b, 0, sizeof(b));
  b.h.ver        = RF_PROTO_VER;
  b.h.flags      = 0;
  b.req_id_le    = req_id;
  memcpy(b.mac, mac6, 6);
  b.uptime_s_le = uptime_s;

  uint8_t payload[sizeof(rf_body_G_t) + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u)];
  size_t out_len = 0;

  if (!rf_encode_payload((uint8_t)RF_MSG_G_CFG_REQ,
                         &b, sizeof(b),
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }

  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_G_CFG_REQ, payload, (uint8_t)out_len, user);
}

// ============================================================
// Node helper: send 'T' Telemetry
// ============================================================
bool rf_node_send_T(
    rf_send_fn_t send_fn,
    void* user,
    uint16_t req_id,
    uint8_t health_flags,
    const uint8_t mac6[6],
    uint32_t uptime_s,
    const uint8_t* tlv,
    uint8_t tlv_len) {

  if (!send_fn || !mac6) return false;
  if (tlv_len > 0 && !tlv) return false;

  const size_t body_len = sizeof(rf_body_T_hdr_t) + (size_t)tlv_len;
  if (!rf_msg_body_len_ok((uint8_t)RF_MSG_T_TELEMETRY, body_len)) return false;

  uint8_t body[RF_MAX_BODY_LEN];
  memset(body, 0, sizeof(body));

  rf_body_T_hdr_t h;
  memset(&h, 0, sizeof(h));
  h.h.ver        = RF_PROTO_VER;
  h.h.flags      = health_flags;
  h.req_id_le    = req_id;
  memcpy(h.mac, mac6, 6);
  h.uptime_s_le = uptime_s;
  h.tlv_len      = tlv_len;

  memcpy(body, &h, sizeof(h));

  if (tlv_len) {
    memcpy(body + sizeof(rf_body_T_hdr_t), tlv, tlv_len);
  }

  uint8_t payload[RF_MAX_PAYLOAD_LEN];
  size_t out_len = 0;

  if (!rf_encode_payload((uint8_t)RF_MSG_T_TELEMETRY,
                         body, body_len,
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }
  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_T_TELEMETRY, payload, (uint8_t)out_len, user);
}

// ============================================================
// Node helper: send 'H' Heartbeat
// ============================================================
bool rf_node_send_H(
    rf_send_fn_t send_fn,
    void* user,
    uint16_t req_id,
    const uint8_t mac6[6],
    uint32_t uptime_s) {

  if (!send_fn || !mac6) return false;

  rf_body_H_t b;
  memset(&b, 0, sizeof(b));
  b.h.ver        = RF_PROTO_VER;
  b.h.flags      = 0;
  b.req_id_le    = req_id;
  memcpy(b.mac, mac6, 6);
  b.uptime_s_le = uptime_s;

  uint8_t payload[sizeof(rf_body_H_t) + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u)];
  size_t out_len = 0;

  if (!rf_encode_payload((uint8_t)RF_MSG_H_HEARTBEAT,
                         &b, sizeof(b),
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }
  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_H_HEARTBEAT, payload, (uint8_t)out_len, user);
}

// ============================================================
// Master helper: send 'C' Command
// ============================================================
bool rf_master_send_C(
    rf_send_fn_t send_fn,
    void* user,
    uint16_t req_id,
    uint16_t issued_ms16,
    uint16_t ttl_ms,
    uint8_t mode,
    const uint8_t* tlv,
    uint8_t tlv_len) {

  if (!send_fn) return false;
  if (tlv_len > 0 && !tlv) return false;

  const size_t body_len = sizeof(rf_body_C_hdr_t) + (size_t)tlv_len;
  if (!rf_msg_body_len_ok((uint8_t)RF_MSG_C_CMD, body_len)) return false;

  uint8_t body[RF_MAX_BODY_LEN];
  memset(body, 0, sizeof(body));

  rf_body_C_hdr_t h;
  memset(&h, 0, sizeof(h));
  h.h.ver          = RF_PROTO_VER;
  h.h.flags        = 0;
  h.req_id_le      = req_id;
  h.issued_ms16_le = issued_ms16;
  h.ttl_ms_le      = ttl_ms;
  h.mode           = mode;
  h.tlv_len        = tlv_len;

  memcpy(body, &h, sizeof(h));

  if (tlv_len) {
    memcpy(body + sizeof(rf_body_C_hdr_t), tlv, tlv_len);
  }

  uint8_t payload[RF_MAX_PAYLOAD_LEN];
  size_t out_len = 0;

  if (!rf_encode_payload((uint8_t)RF_MSG_C_CMD,
                         body, body_len,
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }

  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_C_CMD, payload, (uint8_t)out_len, user);
}

// ============================================================
// Node helper: send 'Q' ping
// ============================================================
bool rf_node_send_Q(
    rf_send_fn_t send_fn, void* user,
    uint16_t req_id,
    uint32_t nonce) {

  if (!send_fn) return false;

  rf_body_Q_t b;
  memset(&b, 0, sizeof(b));
  b.h.ver   = RF_PROTO_VER;
  b.h.flags = 0;
  b.req_id_le = req_id;
  b.nonce_le  = nonce;

  uint8_t payload[sizeof(rf_body_Q_t) + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u)];
  size_t out_len = 0;
  if (!rf_encode_payload((uint8_t)RF_MSG_Q_PING,
                         &b, sizeof(b),
                         payload, sizeof(payload),
                         &out_len)) {
    return false;
  }
  if (out_len == 0 || out_len > 255) return false;

  return send_fn((uint8_t)RF_MSG_Q_PING, payload, (uint8_t)out_len, user);
}

// ============================================================
// Master helper: 'P' -> JSON
// ============================================================
static void mac6_to_hex12(const uint8_t mac[6], char out13[13]) {
  static const char* hex = "0123456789ABCDEF";
  for (int i = 0; i < 6; i++) {
    out13[i*2+0] = hex[(mac[i] >> 4) & 0x0F];
    out13[i*2+1] = hex[(mac[i] >> 0) & 0x0F];
  }
  out13[12] = '\0';
}

// ===========================================================
bool rf_master_P_to_json(const rf_body_P_t* p, const char* type_str, char* json_out, size_t json_cap) {
  if (!p || !json_out || json_cap == 0) return false;

  const char* t = (type_str && type_str[0]) ? type_str : "provision_one";

  char uid[13];
  mac6_to_hex12(p->mac, uid);

  const unsigned model_id = (unsigned)p->model_id_le;

  int w = snprintf(
      json_out, json_cap,
      "{\"type\":\"%s\",\"device_uid\":\"%s\",\"model_id\":%u}",
      t, uid, model_id);

  return (w > 0) && ((size_t)w < json_cap);
}

// ===========================================================
bool rf_encode_payload(
    uint8_t msg_type,
    const void* body, size_t body_len,
    uint8_t* out, size_t out_cap,
    size_t* out_len) {

  if (!body || !out || !out_len) return false;

  if (!rf_msg_body_len_ok(msg_type, body_len)) return false;

  const size_t need = body_len + (RF_PROTO_USE_APP_CRC8 ? 1u : 0u);
  if (out_cap < need) return false;

  memcpy(out, body, body_len);

  if (RF_PROTO_USE_APP_CRC8) {
    uint8_t tmp[1 + 32];
    if (body_len > 32) return false;
    tmp[0] = msg_type;
    memcpy(&tmp[1], out, body_len);
    out[body_len] = rf_crc8_maxim(tmp, 1 + body_len);
  }

  *out_len = need;
  return true;
}

// ===========================================================
rf_decode_status_t rf_decode_payload(
    uint8_t msg_type,
    const uint8_t* payload, size_t payload_len,
    const uint8_t** out_body, size_t* out_body_len) {

  if (!payload || !out_body || !out_body_len) return RF_DECODE_BAD_LEN;

  const size_t crc_len = (RF_PROTO_USE_APP_CRC8 ? 1u : 0u);
  if (payload_len < crc_len) return RF_DECODE_BAD_LEN;
  const size_t body_len = payload_len - crc_len;
  if (!rf_msg_body_len_ok(msg_type, body_len)) return RF_DECODE_BAD_LEN;

  if (RF_PROTO_USE_APP_CRC8) {
    uint8_t tmp[1 + 32];
    if (body_len > 32) return RF_DECODE_BAD_LEN;
    tmp[0] = msg_type;
    memcpy(&tmp[1], payload, body_len);
    const uint8_t crc_calc = rf_crc8_maxim(tmp, 1 + body_len);
    const uint8_t crc_rx   = payload[body_len];
    if (crc_calc != crc_rx) return RF_DECODE_BAD_CRC;
  }

  const rf_proto_hdr_t* h = (const rf_proto_hdr_t*)payload;
  if (h->ver != RF_PROTO_VER) return RF_DECODE_BAD_PROTO;

  *out_body = payload;
  *out_body_len = body_len;
  return RF_DECODE_OK;
}

// ===========================================================
rf_decode_status_t rf_decode_T(
    const uint8_t* payload, size_t payload_len,
    rf_T_view_t* out) {

  if (!out) return RF_DECODE_BAD_LEN;

  const uint8_t* body = nullptr;
  size_t body_len = 0;

  rf_decode_status_t st = rf_decode_payload(
      (uint8_t)RF_MSG_T_TELEMETRY,
      payload, payload_len,
      &body, &body_len);
  if (st != RF_DECODE_OK) return st;

  if (body_len < sizeof(rf_body_T_hdr_t)) return RF_DECODE_BAD_LEN;

  const rf_body_T_hdr_t* h = (const rf_body_T_hdr_t*)body;

  const size_t tlv_len = (size_t)h->tlv_len;
  const size_t expect_len = sizeof(rf_body_T_hdr_t) + tlv_len;
  if (expect_len != body_len) return RF_DECODE_BAD_LEN;

  out->req_id       = h->req_id_le;
  out->health_flags = h->h.flags;
  memcpy(out->mac, h->mac, 6);
  out->uptime_s    = h->uptime_s_le;
  out->tlv_len      = (uint8_t)tlv_len;
  out->tlv          = (tlv_len ? (body + sizeof(rf_body_T_hdr_t)) : nullptr);

  return RF_DECODE_OK;
}

// ===========================================================
rf_decode_status_t rf_decode_G(
    const uint8_t* payload, size_t payload_len,
    rf_G_view_t* out) {

  if (!out) return RF_DECODE_BAD_LEN;

  const uint8_t* body = nullptr;
  size_t body_len = 0;

  rf_decode_status_t st = rf_decode_payload(
      (uint8_t)RF_MSG_G_CFG_REQ,
      payload, payload_len,
      &body, &body_len);
  if (st != RF_DECODE_OK) return st;

  if (body_len != sizeof(rf_body_G_t)) return RF_DECODE_BAD_LEN;

  const rf_body_G_t* h = (const rf_body_G_t*)body;

  out->req_id    = h->req_id_le;
  memcpy(out->mac, h->mac, 6);
  out->uptime_s = h->uptime_s_le;

  return RF_DECODE_OK;
}

// ===========================================================
rf_decode_status_t rf_decode_H(
    const uint8_t* payload, size_t payload_len,
    rf_H_view_t* out) {

  if (!out) return RF_DECODE_BAD_LEN;

  const uint8_t* body = nullptr;
  size_t body_len = 0;

  rf_decode_status_t st = rf_decode_payload(
      (uint8_t)RF_MSG_H_HEARTBEAT,
      payload, payload_len,
      &body, &body_len);
  if (st != RF_DECODE_OK) return st;

  if (body_len != sizeof(rf_body_H_t)) return RF_DECODE_BAD_LEN;

  const rf_body_H_t* h = (const rf_body_H_t*)body;

  out->req_id    = h->req_id_le;
  memcpy(out->mac, h->mac, 6);
  out->uptime_s = h->uptime_s_le;

  return RF_DECODE_OK;
}

// ===========================================================
rf_decode_status_t rf_decode_C(
    const uint8_t* payload, size_t payload_len,
    rf_C_view_t* out) {

  if (!out) return RF_DECODE_BAD_LEN;

  const uint8_t* body = nullptr;
  size_t body_len = 0;

  rf_decode_status_t st = rf_decode_payload(
      (uint8_t)RF_MSG_C_CMD,
      payload, payload_len,
      &body, &body_len);
  if (st != RF_DECODE_OK) return st;

  if (body_len < sizeof(rf_body_C_hdr_t)) return RF_DECODE_BAD_LEN;

  const rf_body_C_hdr_t* h = (const rf_body_C_hdr_t*)body;

  const size_t tlv_len = (size_t)h->tlv_len;
  const size_t expect_len = sizeof(rf_body_C_hdr_t) + tlv_len;
  if (expect_len != body_len) return RF_DECODE_BAD_LEN;

  out->req_id      = h->req_id_le;
  out->issued_ms16 = h->issued_ms16_le;
  out->ttl_ms      = h->ttl_ms_le;
  out->mode        = h->mode;
  out->tlv_len     = (uint8_t)tlv_len;
  out->tlv         = (tlv_len ? (body + sizeof(rf_body_C_hdr_t)) : nullptr);

  return RF_DECODE_OK;
}

// ===========================================================
bool rf_parse_hwcfg_res_w(
    const uint8_t* body, size_t body_len,
    rf_hwcfg_res_view_t* out) {

  if (!body || !out) return false;
  if (body_len < sizeof(rf_body_w_hdr_t)) return false;

  const rf_body_w_hdr_t* h = (const rf_body_w_hdr_t*)body;

  const size_t tlv_len = (size_t)h->tlv_len;
  if (sizeof(rf_body_w_hdr_t) + tlv_len != body_len) return false;

  out->req_id      = h->req_id_le;
  out->result      = h->result;
  out->reason      = h->reason;
  out->hw_cfg_crc8 = h->hw_cfg_crc8;
  out->tlv_len     = (uint8_t)tlv_len;
  out->tlv         = (tlv_len ? (body + sizeof(rf_body_w_hdr_t)) : nullptr);
  return true;
}

