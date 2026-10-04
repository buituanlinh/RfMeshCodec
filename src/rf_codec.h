#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "rf_proto_map.h"

#ifdef __cplusplus
extern "C"
{
#endif

  // ===========================================================
  // Legacy-compatible decode/encode for rfmesh_codec_tick()
  typedef enum : uint8_t
  {
    RF_DECODE_OK = 0,
    RF_DECODE_BAD_LEN,
    RF_DECODE_BAD_PROTO,
    RF_DECODE_BAD_CRC,
  } rf_decode_status_t;

  // ============================================================
  // TYPE DEFINE STRUCTURE
  // SCOPE
  // ============================================================
  typedef struct
  {
    uint16_t req_id;
    uint8_t health_flags; // = rf_proto_hdr_t.flags
    uint8_t mac[6];
    uint32_t uptime_s;
    const uint8_t *tlv; // trỏ vào buffer payload đã decode
    uint8_t tlv_len;
  } rf_T_view_t;

  typedef struct
  {
    uint16_t req_id;
    uint8_t mac[6];
    uint32_t uptime_s;
  } rf_G_view_t;

  typedef struct
  {
    uint16_t req_id;
    uint16_t issued_ms16;
    uint16_t ttl_ms;
    uint8_t mode;
    const uint8_t *tlv;
    uint8_t tlv_len;
  } rf_C_view_t;

  typedef struct
  {
    uint16_t req_id;
    uint8_t mac[6];
    uint32_t uptime_s;
  } rf_H_view_t;

  typedef struct
  {
    uint8_t *buf;
    uint8_t cap;
    uint8_t len;
  } rf_tlv_writer_t;

  typedef struct
  {
    uint16_t req_id;
    uint8_t result;
    uint8_t reason;
    uint8_t hw_cfg_crc8;
    const uint8_t *tlv;
    uint8_t tlv_len;
  } rf_hwcfg_res_view_t;

  // ============================================================
  // CRC-8/MAXIM (poly 0x31, refin/refout true; reversed poly 0x8C)
  // init=0x00, xorout=0x00
  // ============================================================
  uint8_t rf_crc8_maxim(const uint8_t *data, size_t len);

  // ============================================================
  // Generic send hook (codec không phụ thuộc RF24Mesh/RF24Network)
  // msg_type sẽ được gán vào RF24NetworkHeader.type ở tầng gọi.
  // ============================================================
  typedef bool (*rf_send_fn_t)(uint8_t msg_type, const void *buf, uint8_t len, void *user);

  // ============================================================
  // Frame 'P' (provision_req)
  // Body layout: rf_body_P_t (12B) [+ crc8 1B nếu enable]
  // crc8 tính trên: [msg_type][body]
  // ============================================================

  // Node helper: build + send provision_req 'P'
  bool rf_node_send_P_once(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      const uint8_t mac6[6],
      uint16_t model_id);

  // Node helper: build + send hw_cfg_req 'W'
  bool rf_node_send_W(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      const uint8_t mac6[6],
      uint32_t uptime_s);

  // Node helper: build + send role_cfg_req 'G'
  bool rf_node_send_G(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      const uint8_t mac6[6],
      uint32_t uptime_s);

  // Node helper: build + send Telemetry 'T'
  // - health_flags nằm trong rf_proto_hdr_t.flags (1B)
  // - tlv_len là số byte TLV
  bool rf_node_send_T(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      uint8_t health_flags,
      const uint8_t mac6[6],
      uint32_t uptime_s,
      const uint8_t *tlv,
      uint8_t tlv_len);

  // Node helper: build + send Heartbeat 'H'
  bool rf_node_send_H(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      const uint8_t mac6[6],
      uint32_t uptime_s);

  // Node helper: build + send ping 'Q'
  bool rf_node_send_Q(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      uint32_t nonce);

  // Master helper: convert body 'P' -> JSON payload để publish sang bridge
  // JSON tối thiểu:
  // {"type":"provision_one","device_uid":"AABBCCDDEEFF","model_id":1001}
  bool rf_master_P_to_json(
      const rf_body_P_t *p,
      const char *type_str,
      char *json_out, size_t json_cap);

  // Hàm cốt lõi
  bool rf_encode_payload(
      uint8_t msg_type,
      const void *body, size_t body_len,
      uint8_t *out, size_t out_cap,
      size_t *out_len);

  // Hàm cốt lõi
  rf_decode_status_t rf_decode_payload(
      uint8_t msg_type,
      const uint8_t *payload, size_t payload_len,
      const uint8_t **out_body, size_t *out_body_len);

  // Decode+parse type 'T'
  rf_decode_status_t rf_decode_T(
      const uint8_t *payload, size_t payload_len,
      rf_T_view_t *out);

  // Decode+parse type 'G'
  rf_decode_status_t rf_decode_G(
      const uint8_t *payload, size_t payload_len,
      rf_G_view_t *out);

  // Master helper: build command 'C'
  // - C layout: rf_body_C_hdr_t + TLV command bytes + app_crc8
  // - tlv_len là số byte TLV command
  bool rf_master_send_C(
      rf_send_fn_t send_fn,
      void *user,
      uint16_t req_id,
      uint16_t issued_ms16,
      uint16_t ttl_ms,
      uint8_t mode,
      const uint8_t *tlv,
      uint8_t tlv_len);

  // Decode+parse type 'C'
  rf_decode_status_t rf_decode_C(
      const uint8_t *payload, size_t payload_len,
      rf_C_view_t *out);

  // Decode+parse type 'H'
  rf_decode_status_t rf_decode_H(
      const uint8_t *payload, size_t payload_len,
      rf_H_view_t *out);

  // ===========================================================
  // Telemetry TLV writer (Node side)
  // - Build TLV stream for msg 'T'
  // - TLV format: [Type:1B][Len:1B][Value:LenB]
  // - Multi-byte values are little-endian
  // ===========================================================
  static inline void rf_tlvw_init(rf_tlv_writer_t *w, uint8_t *buf, uint8_t cap)
  {
    w->buf = buf;
    w->cap = cap;
    w->len = 0;
  }

  static inline uint8_t rf_tlvw_len(const rf_tlv_writer_t *w) { return w->len; }

  static inline bool rf_tlvw_put_bytes(rf_tlv_writer_t *w, uint8_t type, const void *v, uint8_t vlen)
  {
    const uint16_t need = (uint16_t)w->len + 2u + (uint16_t)vlen;
    if (need > (uint16_t)w->cap)
      return false;

    w->buf[w->len++] = type;
    w->buf[w->len++] = vlen;

    const uint8_t *p = (const uint8_t *)v;
    for (uint8_t i = 0; i < vlen; i++)
      w->buf[w->len++] = p[i];
    return true;
  }

  static inline bool rf_tlvw_put_u8(rf_tlv_writer_t *w, uint8_t type, uint8_t v)
  {
    return rf_tlvw_put_bytes(w, type, &v, 1);
  }

  static inline bool rf_tlvw_put_u16_le(rf_tlv_writer_t *w, uint8_t type, uint16_t v)
  {
    uint8_t b[2];
    b[0] = (uint8_t)(v & 0xFFu);
    b[1] = (uint8_t)((v >> 8) & 0xFFu);
    return rf_tlvw_put_bytes(w, type, b, 2);
  }

  static inline bool rf_tlvw_put_i16_le(rf_tlv_writer_t *w, uint8_t type, int16_t v)
  {
    return rf_tlvw_put_u16_le(w, type, (uint16_t)v);
  }

  // ===========================================================
  // Command TLV writer (Master side, msg 'C')
  // - Build TLV stream for msg 'C'
  // - TLV format: [Type:1B][Len:1B][Value:LenB]
  // ===========================================================
  static inline bool rf_cmd_tlvw_put_switch0(rf_tlv_writer_t *w, uint8_t state)
  {
    return rf_tlvw_put_u8(w, (uint8_t)RF_TLV_CMD_SWITCH0_STATE, state ? 1 : 0);
  }

  static inline bool rf_cmd_tlvw_put_switch1(rf_tlv_writer_t *w, uint8_t state)
  {
    return rf_tlvw_put_u8(w, (uint8_t)RF_TLV_CMD_SWITCH1_STATE, state ? 1 : 0);
  }

  static inline bool rf_cmd_tlvw_put_switch2(rf_tlv_writer_t *w, uint8_t state)
  {
    return rf_tlvw_put_u8(w, (uint8_t)RF_TLV_CMD_SWITCH2_STATE, state ? 1 : 0);
  }

  static inline bool rf_cmd_tlvw_put_reboot(rf_tlv_writer_t *w, uint8_t reboot)
  {
    return rf_tlvw_put_u8(w, (uint8_t)RF_TLV_CMD_REBOOT, reboot ? 1 : 0);
  }

  // Servo pulse cho channel idx 0..2; từ chối pulse ngoài 500..2500 (đặc biệt 0).
  static inline bool rf_cmd_tlvw_put_servo_pulse(rf_tlv_writer_t *w, uint8_t idx, uint16_t pulse_us)
  {
    if (idx > 2u)
      return false;
    if (pulse_us < RF_SERVO_PULSE_MIN_US || pulse_us > RF_SERVO_PULSE_MAX_US)
      return false;
    return rf_tlvw_put_u16_le(w, (uint8_t)((uint8_t)RF_TLV_CMD_SERVO0_PULSE + idx), pulse_us);
  }

  // Telemetry servo pulse cho channel idx 0..2 (Node side).
  static inline bool rf_tlvw_put_servo_pulse(rf_tlv_writer_t *w, uint8_t idx, uint16_t pulse_us)
  {
    if (idx > 2u)
      return false;
    return rf_tlvw_put_u16_le(w, (uint8_t)((uint8_t)RF_TLV_TELE_SERVO0_PULSE + idx), pulse_us);
  }

  // DHT22 packed (fixed-point): temp_cC=T*100 (int16), rh_cP=RH*100 (uint16)
  static inline bool rf_tlvw_put_dht22(rf_tlv_writer_t *w, int16_t temp_cC, uint16_t rh_cP)
  {
    uint8_t b[4];
    const uint16_t t = (uint16_t)temp_cC;
    b[0] = (uint8_t)(t & 0xFFu);
    b[1] = (uint8_t)((t >> 8) & 0xFFu);
    b[2] = (uint8_t)(rh_cP & 0xFFu);
    b[3] = (uint8_t)((rh_cP >> 8) & 0xFFu);
    return rf_tlvw_put_bytes(w, (uint8_t)RF_TLV_TELE_DHT22, b, 4);
  }

  // Parse body đã qua rf_decode_payload() (tức là payload đã đúng proto ver + app_crc OK nếu bật)
  bool rf_parse_hwcfg_res_w(
      const uint8_t *body, size_t body_len,
      rf_hwcfg_res_view_t *out);

#ifdef __cplusplus
}
#endif
