#pragma once
#include <Arduino.h>
#include <RF24.h>


// RF24_1MBPS   = 0,  // 1 Mbps
// RF24_2MBPS   = 1,  // 2 Mbps
// RF24_250KBPS = 2   // 250 kbps

// RF24_PA_MIN  = 0,
// RF24_PA_LOW  = 1,
// RF24_PA_HIGH = 2,
// RF24_PA_MAX  = 3,
// RF24_PA_ERROR= 4  // giữ để tương thích ngược, không dùng set


// RF24 params used for bring-up (PHƯƠNG ÁN 1):
// - channel/datarate FIX CỨNG compile-time tại đây
// - msg 'g' KHÔNG được phép override ch/dr

typedef struct {
  uint8_t           channel;
  rf24_datarate_e   datarate;
  rf24_pa_dbm_e     PA;
  uint8_t           retry_delay;   // RF24 retries delay (0..15) => 250us steps
  uint8_t           retry_count;   // RF24 retries count (0..15)
  bool              autoAck;
} rf24_params_t;

// --- DEFAULTS (SỬA Ở ĐÂY KHI MUỐN ĐỔI CHANNEL / DATARATE) ---
static const rf24_params_t RF24_DEFAULT = {
  /*channel*/      100,
  /*datarate*/     RF24_250KBPS,
  /*PA*/           RF24_PA_LOW,
  /*retry_delay*/  15,
  /*retry_count*/  15,
  /*autoAck*/      true,
};

// Mỗi firmware (Master / Node) phải ĐỊNH NGHĨA biến này đúng 1 lần,
// ví dụ trong .ino:
//   rf24_params_t rf24_cfg = RF24_DEFAULT;
extern rf24_params_t rf24_cfg;

// Optional helper to apply config consistently.
static inline void rf24_apply_cfg(RF24& radio, const rf24_params_t& cfg) {
  radio.setChannel(cfg.channel);
  radio.setDataRate(cfg.datarate);
  radio.setPALevel(cfg.PA);
  radio.setRetries(cfg.retry_delay, cfg.retry_count);
  radio.setAutoAck(cfg.autoAck);
}
