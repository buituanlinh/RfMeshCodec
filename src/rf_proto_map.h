#pragma once
#include <stdint.h>
#include <stddef.h>

#include "rf_struct_defs.h"

enum
{
  RF_MAX_PAYLOAD_LEN = 32u,
  RF_APP_CRC_LEN = (RF_PROTO_USE_APP_CRC8 ? 1u : 0u),
  RF_MAX_BODY_LEN = RF_MAX_PAYLOAD_LEN - RF_APP_CRC_LEN,
  RF_T_TELEMETRY_MAX_TLV_LEN = RF_MAX_BODY_LEN - sizeof(rf_body_T_hdr_t),
  RF_C_COMMAND_MAX_TLV_LEN = RF_MAX_BODY_LEN - sizeof(rf_body_C_hdr_t),

  // Mỗi channel chỉ có một TLV (digital 3B hoặc servo 4B); worst case: 3 servo + reboot.
  RF_C_COMMAND_WORST_TLV_LEN = 3u * 4u + 3u
};

static_assert(RF_C_COMMAND_WORST_TLV_LEN <= RF_C_COMMAND_MAX_TLV_LEN,
              "TLV worst case cua msg C vuot gioi han frame");
static_assert(4u <= RF_T_TELEMETRY_MAX_TLV_LEN,
              "TLV servo telemetry (4B) khong vua trong mot frame T");

#ifdef __cplusplus
extern "C"
{
#endif

  // ============================================================
  // Shared lookup tables (Cách A): dùng chung cho Node/Master
  // - Không phụ thuộc DB
  // - Không phụ thuộc RF24Mesh/RF24Network
  // ============================================================

  typedef struct
  {
    uint8_t msg_type;
    uint8_t min_body_len; // body length (không tính CRC8)
    uint8_t max_body_len; // =min nếu frame fixed-length
    const char *name;
  } rf_msg_map_t;

  typedef struct
  {
    uint16_t model_id;
    const char *name;
  } rf_model_map_t;

  // Return NULL nếu không tìm thấy
  const rf_msg_map_t *rf_msg_map_find(uint8_t msg_type);
  const rf_model_map_t *rf_model_map_find(uint16_t model_id);

  // Helpers tiện dùng
  size_t rf_msg_min_body_len(uint8_t msg_type);
  size_t rf_msg_max_body_len(uint8_t msg_type);
  const char *rf_msg_name(uint8_t msg_type);

  // ACK 'A' (Master↔Node) stage/result
  typedef enum : uint8_t
  {
    RF_ACK_STAGE_RECV = 1,
    RF_ACK_STAGE_FINAL = 2,
  } rf_ack_stage_t;

  typedef enum : uint8_t
  {
    RF_ACK_RESULT_NA = 0,
    RF_ACK_RESULT_OK = 1,
    RF_ACK_RESULT_FAIL = 2,
  } rf_ack_result_t;

  const char *rf_ack_stage_name(uint8_t stage);
  const char *rf_ack_result_name(uint8_t result);

  const char *rf_result_name(uint8_t result);
  const char *rf_reason_name(uint8_t reason);
  const char *rf_model_name(uint16_t model_id);

  // Validate body_len (không tính CRC8)
  bool rf_msg_body_len_ok(uint8_t msg_type, size_t body_len);

#ifdef __cplusplus
}
#endif
