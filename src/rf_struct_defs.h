#pragma once
#include <stdint.h>
#include <stddef.h>

#ifndef RF_PROTO_VER
#define RF_PROTO_VER 1
#endif

#ifndef RF_PROTO_USE_APP_CRC8
#define RF_PROTO_USE_APP_CRC8 1
#endif

// Giữ tương thích tick cũ
#ifndef RF_CODEC_PROTO_VER
#define RF_CODEC_PROTO_VER RF_PROTO_VER
#endif

enum rf_msg_type_t : uint8_t
{
  RF_MSG_A_ACK = 'A',           // msg_type cho các gói ACK (đã sử dụng)
  RF_MSG_P_PROVISION_REQ = 'P', // msg_type cho gói provisioning (đã sử dụng)
  RF_MSG_p_PROVISION_RES = 'p', // msg_type cho gói provisioning ack (đã sử dụng)
  RF_MSG_G_CFG_REQ = 'G',       // msg_type cho gói role_config (đã sử dụng)
  RF_MSG_g_CFG_RES = 'g',       // msg_type cho gói role_response (đã sử dụng)
  RF_MSG_W_HWCFG_REQ = 'W',     // msg_type cho gói hw_cfg_req
  RF_MSG_w_HWCFG_RES = 'w',     // msg_type cho gói hw_cfg_res
  RF_MSG_T_TELEMETRY = 'T',     // msg_type cho gói telemetry (đã sử dụng)
  RF_MSG_C_CMD = 'C',           // msg_type cho gói command
  RF_MSG_Q_PING = 'Q',          // msg_type cho gói ping (đã sử dụng)
  RF_MSG_H_HEARTBEAT = 'H',     // msg_type cho gói heartbeat (đã sử dụng)
};

enum rf_result_t : uint8_t
{
  RF_RES_OK = 0,
  RF_RES_ERR = 1,
  RF_RES_RETRY = 2
};
enum rf_reason_t : uint8_t
{
  RF_RSN_NONE = 0,
  RF_RSN_BAD_PROTO = 1,
  RF_RSN_BAD_LEN = 2,
  RF_RSN_EXPIRED = 3,
  RF_RSN_NOT_READY = 4,
  RF_RSN_DENY = 5,
  RF_RSN_BAD_CRC = 6
};

// Runtime state of one RF TX transaction inside scheduler/inflight state machine
typedef enum : uint8_t
{
  RF_IDLE_LISTEN = 0,
  RF_TX_START,
  RF_WAIT_APP_ACK,
  RF_RETRY_BACKOFF,
  RF_DONE
} rf_tx_state_t;

// Logical kind of RF TX item, used for queue classification and debug/logging
typedef enum : uint8_t
{
  RF_TX_KIND_ACK_A = 0,
  RF_TX_KIND_Q_PROV_RESEND_REQ,
  RF_TX_KIND_ROLE_CFG_G,
  RF_TX_KIND_HW_CFG_W,
  RF_TX_KIND_TELEMETRY_T,
  RF_TX_KIND_HEARTBEAT_H,
  RF_TX_KIND_CONTROL_C,
  RF_TX_KIND_PROVISION_P,
  RF_TX_KIND_UNKNOWN
} rf_tx_kind_t;

// Final failure reason of one RF TX transaction
typedef enum : uint8_t
{
  RF_TX_FAIL_NONE = 0,
  RF_TX_FAIL_WRITE_FAIL,
  RF_TX_FAIL_ACK_TIMEOUT,
  RF_TX_FAIL_RETRY_EXHAUSTED,
  RF_TX_FAIL_ACK_MISMATCH,
  RF_TX_FAIL_QUEUE_FULL,
  RF_TX_FAIL_ENCODE_FAIL
} rf_tx_fail_reason_t;

// One pre-encoded RF frame item stored in TX queue, ready for send/retry without rebuilding business payload
typedef struct
{
  rf_tx_kind_t kind;
  uint16_t to_addr;
  uint8_t msg_type;
  uint8_t frame[32];
  uint8_t frame_len;
  bool need_app_ack;
  uint8_t ack_expect_type;
  uint16_t req_id;
  uint8_t max_retries;
  uint32_t ack_timeout_ms;
  uint32_t retry_backoff_ms;
  uint32_t enqueue_ms;
  uint16_t debug_seq;
} rf_tx_item_t;

// One currently in-flight RF transaction being tracked for timeout, retry, ACK match, and final result
typedef struct
{
  bool active;
  rf_tx_item_t item;
  uint32_t first_sent_ms;
  uint32_t last_sent_ms;
  uint8_t retry_count;
  rf_tx_state_t state;
  bool success;
  rf_tx_fail_reason_t fail_reason;
  uint16_t ack_from_addr;
  uint8_t acked_type_rx;
  uint16_t ack_req_id_rx;
} rf_tx_inflight_t;

// ============================================================
// Telemetry TLV types (msg 'T')
// Format: [Type:1B][Len:1B][Value:LenB]
// Endian: Little-endian for multi-byte values
// Reserved range: 0x10..0x2F
// ============================================================
typedef enum : uint8_t
{
  RF_TLV_TELE_DHT22 = 0x10,               // Len=4  : temp_cC(i16) + rh_cP(u16)
  RF_TLV_TELE_RAIN_DO = 0x11,             // Len=1  : state(u8)
  RF_TLV_TELE_RAIN_AO_ADC = 0x12,         // Len=2  : adc(u16)
  RF_TLV_TELE_LIGHT_AO_ADC = 0x13,        // Len=2  : adc(u16)
  RF_TLV_TELE_LIGHT_DO = 0x14,            // Len=1  : state(u8)
  RF_TLV_TELE_LIGHT_GY302 = 0x15,         // Len=2  : lux(u16)
  RF_TLV_TELE_MQ2_AO_ADC = 0x16,          // Len=2  : adc(u16)
  RF_TLV_TELE_MQ7_AO_ADC = 0x17,          // Len=2  : adc(u16)
  RF_TLV_TELE_MQ9_AO_ADC = 0x18,          // Len=2  : adc(u16)
  RF_TLV_TELE_SOIL_AO_ADC = 0x19,         // Len=2  : adc(u16)
  RF_TLV_TELE_SOIL_DO = 0x1A,             // Len=1  : state(u8)
  RF_TLV_TELE_WATER_SW = 0x1B,            // Len=1  : state(u8)
  RF_TLV_TELE_WATER_AO_ADC = 0x1C,        // Len=2  : adc(u16)
  RF_TLV_TELE_WATER_US_MM = 0x1D,         // Len=2  : distance_mm(u16)
  RF_TLV_TELE_TEMP_CONTACT_AO_ADC = 0x1E, // Len=2  : adc(u16)
  RF_TLV_TELE_SWITCH0_STATE = 0x1F,       // Len=1  : state(u8)
  RF_TLV_TELE_SWITCH1_STATE = 0x20,       // Len=1  : state(u8)
  RF_TLV_TELE_SWITCH2_STATE = 0x21,       // Len=1  : state(u8)
  RF_TLV_TELE_SERVO0_PULSE = 0x22,        // Len=2  : pulse_us(u16) thay cho switch_0 khi output là servo
  RF_TLV_TELE_SERVO1_PULSE = 0x23,        // Len=2  : pulse_us(u16) thay cho switch_1 khi output là servo
  RF_TLV_TELE_SERVO2_PULSE = 0x24,        // Len=2  : pulse_us(u16) thay cho switch_2 khi output là servo
} rf_tlv_tele_type_t;

// ============================================================
// Command mode values (msg 'C')
// mode áp dụng cho toàn bộ command frame
// ============================================================
typedef enum : uint8_t
{
  RF_CMD_MODE_NONE = 0,
  RF_CMD_MODE_SET = 1,
  RF_CMD_MODE_TOGGLE = 2,
  RF_CMD_MODE_PULSE = 3,
} rf_cmd_mode_t;

// ============================================================
// Command TLV types (msg 'C')
// Format: [Type:1B][Len:1B][Value:LenB]
// Endian: Little-endian for multi-byte values
// Reserved range: 0x30..0x4F
// ============================================================
typedef enum : uint8_t
{
  RF_TLV_CMD_SWITCH0_STATE = 0x30, // Len=1 : state(u8), 0/1
  RF_TLV_CMD_SWITCH1_STATE = 0x31, // Len=1 : state(u8), 0/1
  RF_TLV_CMD_SWITCH2_STATE = 0x32, // Len=1 : state(u8), 0/1
  RF_TLV_CMD_REBOOT = 0x33,        // Len=1 : reboot(u8), 1=request reboot
  RF_TLV_CMD_SERVO0_PULSE = 0x34,  // Len=2 : pulse_us(u16 LE), 500..2500, thay cho switch_0
  RF_TLV_CMD_SERVO1_PULSE = 0x35,  // Len=2 : pulse_us(u16 LE), 500..2500, thay cho switch_1
  RF_TLV_CMD_SERVO2_PULSE = 0x36,  // Len=2 : pulse_us(u16 LE), 500..2500, thay cho switch_2
} rf_tlv_cmd_type_t;

// Miền pulse servo hợp lệ trên wire; Node còn giới hạn hẹp hơn theo hiệu chuẩn từng kênh.
#define RF_SERVO_PULSE_MIN_US 500u
#define RF_SERVO_PULSE_MAX_US 2500u

typedef enum : uint16_t
{
  RF_MODEL_UNKNOWN = 0x0000,
  RF_MODEL_AVR_ATMEGA328P_16M = 0x0101,
  RF_MODEL_AVR_ATMEGA328P_8M = 0x0102,
  RF_MODEL_AVR_ATMEGA2560_16M = 0x0201,
  RF_MODEL_AVR_ATMEGA32U4_16M = 0x0301,
} rf_model_id_t;

#if defined(__GNUC__)
#define RF_PACKED __attribute__((packed))
#else
#define RF_PACKED
#endif

struct RF_PACKED rf_proto_hdr_t
{
  uint8_t ver;
  uint8_t flags;
};

// 'A' - ACK generic: 2 + 2 + 1 + 4 + 1 + 1 + 4 = 15
struct RF_PACKED AckPacket
{
  rf_proto_hdr_t h;   // ver, flags (2)
  uint16_t req_id_le; // correlate (2)
  uint8_t acked_type; // msg type being acked (e.g. 'T','P','G','Q','W',...) (1)
  uint32_t ms;        // timestamp (4)
  uint8_t stage;      // 1
  uint8_t result;     // 1
  uint32_t rcode;     // 4
}; // total 15

// 'P' (không có DEV_TYPE): 2 + 2 + 6 + 2 = 12B
struct RF_PACKED rf_body_P_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t mac[6];
  uint16_t model_id_le;
}; // total 12B

// 'p' (provision_res): 2 + 2 + 1 + 1 + 2 + 2 = 10B
struct RF_PACKED rf_body_p_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t result;
  uint8_t reason;
  uint16_t retry_after_s_le;
  uint16_t reserved_le;
}; // total 10B

// 'G' (cfg_req): 2 + 2 + 6 + 4 = 14B
struct RF_PACKED rf_body_G_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t mac[6];
  uint32_t uptime_s_le;
}; // total 14B

// 'g' (cfg_res): 2 + 2 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 1 + 2 + 1 = 15B
struct RF_PACKED rf_body_g_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t result;
  uint8_t reason;
  uint8_t rf_role;
  uint8_t rf_addr_mode;
  uint8_t rf_node_id;
  uint16_t rf_address_le;
  uint8_t channel;
  uint8_t datarate;
  uint16_t ttl_s_le;
}; // total 15B

// 'W' (hw_cfg_req): 2 + 2 + 6 + 4 = 14B
struct RF_PACKED rf_body_W_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t mac[6];
  uint32_t uptime_s_le;
}; // total 14B

// 'w' (hw_cfg_res) header: 2 + 2 + 1 + 1 + 1 + 1 = 8B, sau đó TLV bytes (len = tlv_len)
struct RF_PACKED rf_body_w_hdr_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t result;      // rf_result_t
  uint8_t reason;      // rf_reason_t
  uint8_t hw_cfg_crc8; // CRC-8/MAXIM over canonical TLV bytes
  uint8_t tlv_len;     // number of TLV bytes following
}; // total 8B

// 'T' (telemetry) header: 2 + 2 + 6 + 4 + 1 = 15B
// - h.flags dùng làm health_flags
// - uptime_s_le: uptime của node tại thời điểm đóng frame, đơn vị giây
// - tlv_len: số byte TLV theo sau
// - TLV max an toàn lấy từ RF_T_TELEMETRY_MAX_TLV_LEN
struct RF_PACKED rf_body_T_hdr_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t mac[6];
  uint32_t uptime_s_le;
  uint8_t tlv_len;
}; // total 15B

// 'H' (heartbeat): 2 + 2 + 6 + 4 = 14B
struct RF_PACKED rf_body_H_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint8_t mac[6];
  uint32_t uptime_s_le;
}; // total 14B

// 'C' (command TLV): fixed header + TLV payload
// Header size: 2 + 2 + 2 + 2 + 1 + 1 = 10B
// Payload after this header:
//   [TLV...][app_crc8]
struct RF_PACKED rf_body_C_hdr_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint16_t issued_ms16_le;
  uint16_t ttl_ms_le;
  uint8_t mode;
  uint8_t tlv_len;
};

// 'Q' (ping): 2 + 2 + 4 = 8B
struct RF_PACKED rf_body_Q_t
{
  rf_proto_hdr_t h;
  uint16_t req_id_le;
  uint32_t nonce_le;
}; // total 8

#if defined(__GNUC__)
static_assert(sizeof(rf_proto_hdr_t) == 2, "rf_proto_hdr_t size");
static_assert(sizeof(AckPacket) == 15, "AckPacket size mismatch");
static_assert(sizeof(rf_body_P_t) == 12, "rf_body_P_t size mismatch");
static_assert(sizeof(rf_body_p_t) == 10, "rf_body_p_t size mismatch");
static_assert(sizeof(rf_body_G_t) == 14, "rf_body_G_t size mismatch");
static_assert(sizeof(rf_body_g_t) == 15, "rf_body_g_t size mismatch");
static_assert(sizeof(rf_body_W_t) == 14, "rf_body_W_t size mismatch");
static_assert(sizeof(rf_body_w_hdr_t) == 8, "rf_body_w_hdr_t size mismatch");
static_assert(sizeof(rf_body_T_hdr_t) == 15, "rf_body_T_hdr_t size mismatch");
static_assert(sizeof(rf_body_H_t) == 14, "rf_body_H_t size mismatch");
static_assert(sizeof(rf_body_C_hdr_t) == 10, "rf_body_C_hdr_t size mismatch");
static_assert(sizeof(rf_body_Q_t) == 8, "rf_body_Q_t size mismatch");
#endif

#undef RF_PACKED
