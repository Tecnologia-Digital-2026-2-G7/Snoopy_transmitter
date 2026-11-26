#include "buttons.h"
#include "espnow_link.h"
#include "remote_protocol.h"

static const uint8_t RECEIVER_MAC[6] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; // completar MAC del receptor

void setup() {
  Serial.begin(115200);

  if (!espnow_link_begin(RECEIVER_MAC)) {
    Serial.println("Fallo ESP-NOW");
    return;
  }

  if (!buttons_begin()) {
    Serial.println("Fallo al crear botones");
    return;
  }

  for (size_t i = 0; i < buttons_count(); i++) {
    if (!espnow_link_attach(buttons_cmd(i), buttons_semaphore(i))) {
      Serial.println("Fallo al crear tarea de envio");
    }
  }
}

void loop() {
  vTaskDelay(portMAX_DELAY);
}
