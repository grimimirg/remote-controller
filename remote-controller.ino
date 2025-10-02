#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

/*
  Remote Controller (ESP32 + SX127x, 433 MHz)

  Sends two analog channels (Left/Right) read from potentiometers over LoRa,
  at a fixed rate (every 50 ms). Packet format is compact and validated by CRC-8.

  Packet layout (8 bytes total, little-endian for fields >1 byte):
    [0] 0xC3  - magic low (0xA5C3 overall)
    [1] 0xA5  - magic high
    [2] 0x01  - protocol version
    [3] seqLo - sequence number (low byte)
    [4] seqHi - sequence number (high byte)
    [5] L     - left  channel value (0..255)
    [6] R     - right channel value (0..255)
    [7] CRC8  - Dallas/Maxim CRC over bytes [0..6]

  Notes:
  - Uses Sandeep Mistry "LoRa" library.
  - ADC12-bit readings (0..4095) are linearly mapped to 8-bit (0..255).
  - Keep TX rate and bandwidth/legal power consistent with local regulations.
*/

// ==== Pin configuration (adjust to your board/wiring) ====
#define LORA_FREQ_HZ 433E6
#define LORA_SCK     18
#define LORA_MISO    19
#define LORA_MOSI    23
#define LORA_CS       5
#define LORA_RST     14
#define LORA_DIO0    26

#define PIN_POT_LEFT   33   // ADC1, input-only on ESP32
#define PIN_POT_RIGHT  25   // ADC1, input-only on ESP32

// Transmit 20 packets per second (50 ms period)
static const uint32_t TX_PERIOD_MS = 50;

// increments every sent packet
static uint16_t packetSequence = 0;

// ==== CRC-8 Dallas/Maxim (poly 0x31, reflected, init 0x00) ====
// This is a lightweight integrity check suitable for small packets.
static uint8_t crc8Dallas(const uint8_t* data, size_t length) {
  uint8_t crc = 0x00;
  for (size_t byteIndex = 0; byteIndex < length; ++byteIndex) {
    uint8_t currentByte = data[byteIndex];
    for (uint8_t bitIndex = 0; bitIndex < 8; ++bitIndex) {
      uint8_t mix = (crc ^ currentByte) & 0x01;
      crc >>= 1;
      if (mix) crc ^= 0x8C;   // 0x8C is 0x31 reflected
      currentByte >>= 1;
    }
  }
  return crc;
}

// Map 12-bit ADC reading (0..4095) to 8-bit (0..255) with rounding.
static inline uint8_t mapAdc12bitTo8bit(int adcValue) {
  if (adcValue < 0) adcValue = 0;
  if (adcValue > 4095) adcValue = 4095;
  uint32_t s = (uint32_t)adcValue * 255u + 2047u; // rounding
  return (uint8_t)(s / 4095u);                    // 0..255 without overflow
}

void setup() {
  Serial.begin(115200);

  // Wire up the LoRa transceiver (SX1276/77/78/79)
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  // Initialize the radio and basic PHY settings
  if (!LoRa.begin((long)LORA_FREQ_HZ)) {
    // Hard stop if the radio is not found/initialized
    while (true) { delay(1000); }
  }

  Serial.println("LoRa initialization success!");

  //=============================
  // NOTE: Keep these settings aligned with the receiver
  //=============================
  LoRa.setSpreadingFactor(7);       // lower latency, reasonable range
  LoRa.setSignalBandwidth(125E3);   // 125 kHz is a common default
  LoRa.setCodingRate4(5);           // 4/5: moderate robustness
  LoRa.enableCrc();                 // PHY CRC in addition to our payload CRC
  LoRa.setSyncWord(0x12);           // change to isolate from other nodes
  //=============================
  
  LoRa.setTxPower(5);               // Low dBm (1-5) for bench tests. Increase for long range.
}

void loop() {
  // Rate-limit transmissions to TX_PERIOD_MS
  static uint32_t lastTxMillis = 0;
  const uint32_t nowMillis = millis();
  if (nowMillis - lastTxMillis < TX_PERIOD_MS) return;
  lastTxMillis = nowMillis;

  // Sample analog inputs and convert to 8-bit channels
  int leftValue = analogRead(PIN_POT_LEFT);
  int rightValue = analogRead(PIN_POT_RIGHT);
  
  const uint8_t leftChannelValue  = mapAdc12bitTo8bit(leftValue);
  const uint8_t rightChannelValue = mapAdc12bitTo8bit(rightValue);

  // Serial.printf("Left -> %i | Right -> %i\n", leftValue, rightValue);

  // Build the packet (8 bytes)
  uint8_t packet[8];
  packet[0] = 0xC3; packet[1] = 0xA5;      // magic (0xA5C3 little-endian)
  packet[2] = 1;                            // protocol version
  packet[3] = (uint8_t)(packetSequence & 0xFF);
  packet[4] = (uint8_t)(packetSequence >> 8);
  packet[5] = leftChannelValue;
  packet[6] = rightChannelValue;
  packet[7] = crc8Dallas(packet, 7);        // CRC over first 7 bytes

  // Transmit the packet; default endPacket() is blocking until sent
  LoRa.beginPacket();
  LoRa.write(packet, sizeof(packet));
  LoRa.endPacket();

  // Increment sequence (wraps naturally at 65535)
  packetSequence++;
}
