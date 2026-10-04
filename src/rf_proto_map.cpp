#include "rf_proto_map.h"

// NOTE: bảng này là “canonical” cho Cách A.
// Nếu sau này đổi layout (proto v2, thêm trường...), chỉ cần update 1 chỗ.

static const rf_msg_map_t kMsgMap[] = {
  { (uint8_t)RF_MSG_A_ACK,           (uint8_t)sizeof(AckPacket),        (uint8_t)sizeof(AckPacket),       "A/ack" },
  { (uint8_t)RF_MSG_P_PROVISION_REQ, (uint8_t)sizeof(rf_body_P_t),      (uint8_t)sizeof(rf_body_P_t),     "P/provision_req" },
  { (uint8_t)RF_MSG_p_PROVISION_RES, (uint8_t)sizeof(rf_body_p_t),      (uint8_t)sizeof(rf_body_p_t),     "p/provision_res" },
  { (uint8_t)RF_MSG_G_CFG_REQ,       (uint8_t)sizeof(rf_body_G_t),      (uint8_t)sizeof(rf_body_G_t),     "G/cfg_req" },
  { (uint8_t)RF_MSG_g_CFG_RES,       (uint8_t)sizeof(rf_body_g_t),      (uint8_t)sizeof(rf_body_g_t),  		"g/cfg_res" },
	{ (uint8_t)RF_MSG_W_HWCFG_REQ,     (uint8_t)sizeof(rf_body_W_t),      (uint8_t)sizeof(rf_body_W_t),     "W/hwcfg_req" },
  { (uint8_t)RF_MSG_w_HWCFG_RES,     (uint8_t)sizeof(rf_body_w_hdr_t),  32u,                              "w/hwcfg_res" },

  // 'T' variable length: tối thiểu header, tối đa để chừa chỗ cho app payload (không tính CRC8)
  // 32B là giới hạn NRF24 payload, nhưng còn RF24NetworkHeader. Ở codec layer ta chỉ validate mềm.
  { (uint8_t)RF_MSG_T_TELEMETRY,     (uint8_t)sizeof(rf_body_T_hdr_t),  (uint8_t)RF_MAX_BODY_LEN,         "T/telemetry" },
  { (uint8_t)RF_MSG_H_HEARTBEAT,     (uint8_t)sizeof(rf_body_H_t),      (uint8_t)sizeof(rf_body_H_t),     "H/heartbeat" },
	{ (uint8_t)RF_MSG_C_CMD,           (uint8_t)sizeof(rf_body_C_hdr_t),  (uint8_t)RF_MAX_BODY_LEN,         "C/cmd" },
  { (uint8_t)RF_MSG_Q_PING,          (uint8_t)sizeof(rf_body_Q_t),      (uint8_t)sizeof(rf_body_Q_t),     "Q/ping" },
};

static const rf_model_map_t kModelMap[] = {
  { (uint16_t)RF_MODEL_UNKNOWN,            "UNKNOWN" },
  { (uint16_t)RF_MODEL_AVR_ATMEGA328P_16M, "AVR_ATMEGA328P_16M" },
  { (uint16_t)RF_MODEL_AVR_ATMEGA328P_8M,  "AVR_ATMEGA328P_8M" },
  { (uint16_t)RF_MODEL_AVR_ATMEGA2560_16M, "AVR_ATMEGA2560_16M" },
  { (uint16_t)RF_MODEL_AVR_ATMEGA32U4_16M, "AVR_ATMEGA32U4_16M" },
};

const rf_msg_map_t* rf_msg_map_find(uint8_t msg_type) {
  for (size_t i = 0; i < (sizeof(kMsgMap)/sizeof(kMsgMap[0])); i++) {
    if (kMsgMap[i].msg_type == msg_type) return &kMsgMap[i];
  }
  return 0;
}

const rf_model_map_t* rf_model_map_find(uint16_t model_id) {
  for (size_t i = 0; i < (sizeof(kModelMap)/sizeof(kModelMap[0])); i++) {
    if (kModelMap[i].model_id == model_id) return &kModelMap[i];
  }
  return 0;
}

size_t rf_msg_min_body_len(uint8_t msg_type) {
  const rf_msg_map_t* m = rf_msg_map_find(msg_type);
  return m ? (size_t)m->min_body_len : 0u;
}

size_t rf_msg_max_body_len(uint8_t msg_type) {
  const rf_msg_map_t* m = rf_msg_map_find(msg_type);
  return m ? (size_t)m->max_body_len : 0u;
}

const char* rf_msg_name(uint8_t msg_type) {
  const rf_msg_map_t* m = rf_msg_map_find(msg_type);
  return m ? m->name : "?";
}

const char* rf_ack_stage_name(uint8_t stage) {
  switch (stage) {
    case (uint8_t)RF_ACK_STAGE_RECV:  return "RECV";
    case (uint8_t)RF_ACK_STAGE_FINAL: return "FINAL";
    default: return "?";
  }
}

const char* rf_ack_result_name(uint8_t result) {
  switch (result) {
    case (uint8_t)RF_ACK_RESULT_NA:   return "NA";
    case (uint8_t)RF_ACK_RESULT_OK:   return "OK";
    case (uint8_t)RF_ACK_RESULT_FAIL: return "FAIL";
    default: return "?";
  }
}

const char* rf_result_name(uint8_t result) {
  switch (result) {
    case (uint8_t)RF_RES_OK:    return "OK";
    case (uint8_t)RF_RES_ERR:   return "ERR";
    case (uint8_t)RF_RES_RETRY: return "RETRY";
    default: return "?";
  }
}

const char* rf_reason_name(uint8_t reason) {
  switch (reason) {
    case (uint8_t)RF_RSN_NONE:      return "NONE";
    case (uint8_t)RF_RSN_BAD_PROTO: return "BAD_PROTO";
    case (uint8_t)RF_RSN_BAD_LEN:   return "BAD_LEN";
    case (uint8_t)RF_RSN_EXPIRED:   return "EXPIRED";
    case (uint8_t)RF_RSN_NOT_READY: return "NOT_READY";
    case (uint8_t)RF_RSN_DENY:      return "DENY";
    case (uint8_t)RF_RSN_BAD_CRC:   return "BAD_CRC";
    default: return "?";
  }
}

const char* rf_model_name(uint16_t model_id) {
  const rf_model_map_t* m = rf_model_map_find(model_id);
  return m ? m->name : "UNKNOWN";
}

bool rf_msg_body_len_ok(uint8_t msg_type, size_t body_len) {
  const rf_msg_map_t* m = rf_msg_map_find(msg_type);
  if (!m) return false;
  return (body_len >= (size_t)m->min_body_len) && (body_len <= (size_t)m->max_body_len);
}
