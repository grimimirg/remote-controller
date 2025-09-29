 Remote Controller (ESP32 + LoRa)

# Remote Controller (ESP32 + LoRa)

This firmware turns an ESP32 into a simple two-channel handheld controller. It reads two potentiometers (Left/Right), maps their 12-bit ADC values to 8-bit, and transmits them at a fixed rate over LoRa (433 MHz by default). The receiver drives two PWM outputs accordingly.

## Features

*   Fixed 8-byte packet with header and CRC-8 integrity check.
*   20 Hz transmit rate (one packet every 50 ms).
*   Clean, explicit variable names and inline comments.
*   Arduino IDE compatible (ESP32 core v2.x or v3.x).

## Hardware

*   ESP32 DevKit (any common module).
*   LoRa transceiver SX1276/SX1278 (433 MHz module).
*   Two 10 kΩ potentiometers.
*   Jumper wires; shared GND between ESP32 and LoRa module.

## Pinout (default)

| Signal | ESP32 Pin |
| --- | --- |
| LoRa SCK | GPIO 18 |
| LoRa MISO | GPIO 19 |
| LoRa MOSI | GPIO 23 |
| LoRa CS (NSS) | GPIO 5 |
| LoRa RST | GPIO 14 |
| LoRa DIO0 | GPIO 26 |
| Pot LEFT (wiper) | GPIO 34 (ADC1, input-only) |
| Pot RIGHT (wiper) | GPIO 35 (ADC1, input-only) |

Potentiometers: connect one outer leg to 3V3, the other to GND, the wiper to the listed ADC pin. Add a small 100 nF capacitor from each wiper to GND to reduce noise.

## Packet Format (8 bytes)

```
Byte 0: 0xC3   // magic low   (overall 0xA5C3, little-endian)
Byte 1: 0xA5   // magic high
Byte 2: 0x01   // protocol version
Byte 3: seqLo  // sequence number low
Byte 4: seqHi  // sequence number high
Byte 5: L      // left  channel (0..255)
Byte 6: R      // right channel (0..255)
Byte 7: CRC8   // Dallas/Maxim over bytes 0..6
```

## Quick Start (Arduino IDE)

1.  Install the ESP32 board support (Boards Manager: "esp32 by Espressif").
2.  Install the library **LoRa** by Sandeep Mistry (Library Manager).
3.  Open `remote-controller.ino`.
4.  Select your ESP32 board and the correct serial port.
5.  Upload the sketch and open the Serial Monitor at 115200 baud (optional).

## Configuration

Edit these constants at the top of the sketch to match your wiring and region:

```
#define LORA_FREQ_HZ 433E6    // set to your band (e.g., 433E6 or 868E6)
#define LORA_SCK     18
#define LORA_MISO    19
#define LORA_MOSI    23
#define LORA_CS       5
#define LORA_RST     14
#define LORA_DIO0    26

#define PIN_POT_LEFT   34
#define PIN_POT_RIGHT  35

static const uint32_t TX_PERIOD_MS = 50;  // transmit every 50 ms
```

Radio PHY defaults: `SF7`, `BW=125 kHz`, `CR=4/5`, `syncWord=0x12`. Keep them identical on the receiver.

## How It Works

1.  Each loop cycle (every 50 ms) the firmware reads both ADCs.
2.  ADC 12-bit values (0..4095) are linearly mapped to 8-bit (0..255).
3.  An 8-byte packet is assembled and a CRC-8 is computed over bytes 0..6.
4.  The packet is sent via LoRa; the receiver validates it and applies the values.

## Troubleshooting

*   **No radio detected:** check SPI pins and `LoRa.setPins()` mapping.
*   **Values jumpy:** add 100 nF from each wiper to GND; ensure solid 3V3.
*   **No response on receiver:** verify frequency and PHY parameters match, as well as `syncWord`.

## Regulatory Notes

LoRa modulation, frequency, output power, and duty-cycle must comply with your local regulations. 433 MHz and 868 MHz bands have region-specific limits. Adjust settings accordingly.
