#include <SPI.h>
#include <LoRa.h>

// --- Controllo ---
#define LEFT_PIN    33
#define RIGHT_PIN   25

// --- LoRa ---
#define LORA_SS     5
#define LORA_RST    14
#define LORA_DIO0   2

#define LORA_FREQ   868E6
#define SYNC_WORD   0xF0

//------------------------------------------------------------------------------------------------

void setupLoRa() {
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("LoRa init failed");
    while (true) { }
  }

  LoRa.setSyncWord(SYNC_WORD);
  Serial.println("LoRa ready!");
  delay(1000);
}

void sendLoRaPayload(float leftPercentage, float rightPercentage) {
  String jsonPayload = String("{")
                     + "\"to\":\"receiver\","
                     + "\"left\":" + String(leftPercentage, 2) + ","
                     + "\"right\":" + String(rightPercentage, 2)
                     + "}";

  LoRa.beginPacket();
  LoRa.print(jsonPayload);
  LoRa.endPacket();

  Serial.println("Payload sent:");
  Serial.println(jsonPayload);
}

//------------------------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  setupLoRa();
}

void loop() {
  int analogReadLeft  = analogRead(LEFT_PIN);
  int analogReadRight = analogRead(RIGHT_PIN);

  float percentageLeft  = (analogReadLeft  / 4095.0f) * 100.0f;
  float percentageRight = (analogReadRight / 4095.0f) * 100.0f;

  Serial.printf("Left = %.2f || Right = %.2f\n", percentageLeft, percentageRight);

  sendLoRaPayload(percentageLeft, percentageRight);

  delay(200);
}
