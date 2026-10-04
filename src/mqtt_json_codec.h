#pragma once

#include <Arduino.h>
#include "mqtt_struct_defs.h"

String json_get_type(const String& s);
long json_get_int(const String& s, const char* key, long defVal);
String json_get_str(const String& s, const char* key);
bool json_get_bool(const String& s, const char* key, bool defVal);

bool mqtt_encode_telemetry_frame(const tele_frame_t& f, String& out);
bool mqtt_decode_telemetry_ack(const String& msg, tele_ack_t& out);
bool mqtt_decode_heartbeat_ack(const String& msg, heartbeat_ack_t& out);