#pragma once

#include <Arduino.h>
#include <stdint.h>

struct tele_point_t {
  uint8_t point_id  = 0;   // map chuẩn phía bridge
  uint8_t vt        = 0;   // 0=num, 1=bool
  int32_t v         = 0;   // giá trị đã scale ngầm theo point
};

struct tele_frame_t {
  String   device_uid;
  uint16_t from_node        = 0;
  uint16_t req_id           = 0;
  uint32_t ts               = 0;
  uint32_t uptime_s         = 0;
  tele_point_t points[18];
  uint8_t      point_count  = 0;
};

enum broker_out_kind_t : uint8_t {
  BROKER_OUT_NONE          = 0,
  BROKER_OUT_TELEMETRY     = 1,
  BROKER_OUT_HEARTBEAT     = 2,
  BROKER_OUT_PROVISION_ONE = 3,
  BROKER_OUT_COMMAND_ACK   = 4
};

struct provision_one_frame_t {
  String   device_uid;
  String   device_model;
  uint16_t from_node = 0;
  uint16_t req_id    = 0;
};

struct heartbeat_frame_t {
  String   device_uid;
  uint16_t from_node  = 0;
  uint16_t req_id     = 0;
  uint32_t uptime_s   = 0;
};

struct command_ack_frame_t {
  String   batch_id;
  uint16_t guest_id       = 0;
  uint16_t location_id    = 0;
  uint16_t items_count    = 0;
  int      bridge_ack     = 0;
  String   ack_op;
  String   ack_error;
  uint32_t ack_ts         = 0;
};

struct role_cfg_frame_t {
  String   device_uid;
  uint16_t from_node     = 0;
  uint16_t req_id        = 0;
  uint32_t uptime_s      = 0;
  bool     has_rf_meta   = false;
};

typedef struct {
  broker_out_kind_t      kind = BROKER_OUT_NONE;
  tele_frame_t           tele;
  heartbeat_frame_t      heartbeat;
  provision_one_frame_t  provision;
  command_ack_frame_t    command_ack;
} broker_out_item_t;

typedef struct {
  bool              active = false;
  broker_out_item_t item;
  uint8_t           retry_count = 0;
  uint32_t          sent_ms = 0;
} broker_out_inflight_t;

struct tele_ack_t {
  String   device_uid;
  uint16_t from_node    = 0;
  uint16_t req_id       = 0;
  int      bridge_ack   = 0;
  long     points_count = 0;
  String   ack_op;
  String   ack_error;
};

struct heartbeat_ack_t {
  String   device_uid;
  uint16_t from_node    = 0;
  uint16_t req_id       = 0;
  int      bridge_ack   = 0;
  String   ack_op;
  String   ack_error;
};