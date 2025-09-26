#include <LoRa.h>

// --- LoRa ---
#define LEFT_PIN 33
#define RIGHT_PIN 25

#define LORA_SS     18
#define LORA_RST    14
#define LORA_DIO0   26
#define LORA_FREQ   868E6 


void setup() {
  Serial.begin(115200);
}

void loop() {
  int analogReadLeft = analogRead(LEFT_PIN);
  int analogReadRight = analogRead(RIGHT_PIN);

  Serial.printf("Left = %d || Right = %d\n", analogReadLeft, analogReadRight);

  delay(200);
}
