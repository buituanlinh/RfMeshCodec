#include "mqtt_json_codec.h"

bool mqtt_encode_telemetry_frame(const tele_frame_t& f, String& out) {
  out = "";
  out.reserve(768);

  out += F("{\"type\":\"telemetry_frame\"");

  out += F(",\"device_uid\":\"");
  out += f.device_uid;
  out += F("\"");

  out += F(",\"from_node\":");
  out += (unsigned)f.from_node;

  out += F(",\"req_id\":");
  out += (unsigned)f.req_id;

  out += F(",\"ts\":");
  out += (unsigned long)f.ts;

  out += F(",\"uptime_s\":");
  out += (unsigned long)f.uptime_s;

  out += F(",\"points\":[");

  for (uint8_t i = 0; i < f.point_count; i++) {
    if (i) out += ',';
    out += '{';

    out += F("\"point_id\":");
    out += (unsigned)f.points[i].point_id;

    out += F(",\"vt\":");
    out += (unsigned)f.points[i].vt;

    out += F(",\"v\":");
    out += (long)f.points[i].v;

    out += '}';
  }

  out += F("]}");
  return true;
}

bool mqtt_decode_telemetry_ack(const String& msg, tele_ack_t& out) {
  const String type = json_get_type(msg);
  if (type != "telemetry_ack") return false;

  out.device_uid   = json_get_str(msg, "device_uid");
  out.from_node    = (uint16_t)json_get_int(msg, "from_node", 0);
  out.req_id       = (uint16_t)json_get_int(msg, "req_id", 0);
  out.bridge_ack   = (int)json_get_int(msg, "bridge_ack", 0);
  out.points_count = json_get_int(msg, "points_count", 0);
  out.ack_op       = json_get_str(msg, "ack_op");
  out.ack_error    = json_get_str(msg, "ack_error");
  return true;
}

bool mqtt_decode_heartbeat_ack(const String& msg, heartbeat_ack_t& out) {
  const String type = json_get_type(msg);
  if (type != "heartbeat_ack") return false;

  out.device_uid = json_get_str(msg, "device_uid");
  out.from_node  = (uint16_t)json_get_int(msg, "from_node", 0);
  out.req_id     = (uint16_t)json_get_int(msg, "req_id", 0);
  out.bridge_ack = (int)json_get_int(msg, "bridge_ack", 0);
  out.ack_op     = json_get_str(msg, "ack_op");
  out.ack_error  = json_get_str(msg, "ack_error");
  return true;
}

// Lấy giá trị "type" từ JSON dạng String
String json_get_type(const String& s) {
  const String pattern = F("\"type\":\"");
  int pos = s.indexOf(pattern);
  if (pos < 0) return String();
  pos += pattern.length();
  int end = s.indexOf('"', pos);
  if (end < 0) return String();
  return s.substring(pos, end);
}

// Đọc giá trị số nguyên: "key":123
long json_get_int(const String& s, const char* key, long defVal) {
  String pattern = "\"";
  pattern += key;
  pattern += "\":";
  int pos = s.indexOf(pattern);
  if (pos < 0) return defVal;
  pos += pattern.length();

  int end = pos;
  while (end < (int)s.length() && s[end] != ',' && s[end] != '}') {
    end++;
  }

  String num = s.substring(pos, end);
  num.trim();
  if (!num.length()) return defVal;

  // Nếu ký tự đầu không phải chữ số hoặc dấu '-' -> coi như không hợp lệ
  char c = num[0];
  bool isDigit = (c >= '0' && c <= '9');
  bool isSign  = (c == '-');
  if (!isDigit && !isSign) {return defVal;}

  return num.toInt();
}

// Đọc giá trị bool: "key":true/false hoặc "key":1/0
bool json_get_bool(const String& s, const char* key, bool defVal) {
  String pattern = "\"";
  pattern += key;
  pattern += "\":";
  int pos = s.indexOf(pattern);
  if (pos < 0) return defVal;
  pos += pattern.length();

  // skip spaces (đề phòng)
  while (pos < (int)s.length() && (s[pos] == ' ' || s[pos] == '\t' || s[pos] == '\r' || s[pos] == '\n')) pos++;

  // true/false
  if (s.startsWith(F("true"), pos))  return true;
  if (s.startsWith(F("false"), pos)) return false;

  // 1/0
  if (pos < (int)s.length()) {
    if (s[pos] == '1') return true;
    if (s[pos] == '0') return false;
  }
  return defVal;
}

// Đọc giá trị chuỗi: "key":"value"
String json_get_str(const String& s, const char* key) {
  String pattern = "\"";
  pattern += key;
  pattern += "\":\"";     // "key":"
  int pos = s.indexOf(pattern);
  if (pos < 0) return String();
  pos += pattern.length();

  int end = s.indexOf('"', pos);
  if (end < 0) return String();

  return s.substring(pos, end);
}



