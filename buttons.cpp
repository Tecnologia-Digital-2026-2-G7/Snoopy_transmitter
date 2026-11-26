#include "buttons.h"

struct ButtonDef {
  uint8_t pin;
  RemoteCmd cmd;
};

//pines
static const ButtonDef BUTTONS[] = {
  { 32, CMD_NEXT },
  { 33, CMD_PREVIOUS },
  { 25, CMD_VOLUME_UP },
  { 26, CMD_VOLUME_DOWN },
  { 27, CMD_PLAY_STOP },
};
static const size_t N_BUTTONS = sizeof(BUTTONS) / sizeof(BUTTONS[0]);
static const uint32_t SAMPLE_MS = 10;

static SemaphoreHandle_t s_semaphores[N_BUTTONS];

static void buttons_task(void *) {
  bool stableState[N_BUTTONS];
  bool previousReading[N_BUTTONS];
  for (size_t i = 0; i < N_BUTTONS; i++) {
    stableState[i] = HIGH;
    previousReading[i] = HIGH;
  }

  for (;;) {
    for (size_t i = 0; i < N_BUTTONS; i++) {
      bool currentState = digitalRead(BUTTONS[i].pin);

      if (currentState == previousReading[i]) {
        if (stableState[i] == HIGH && currentState == LOW) {
          xSemaphoreGive(s_semaphores[i]);
        }
        stableState[i] = currentState;
      }
      previousReading[i] = currentState;
    }
    vTaskDelay(pdMS_TO_TICKS(SAMPLE_MS));
  }
}

bool buttons_begin() {
  for (size_t i = 0; i < N_BUTTONS; i++) {
    s_semaphores[i] = xSemaphoreCreateBinary();
    if (s_semaphores[i] == NULL) return false;
    pinMode(BUTTONS[i].pin, INPUT_PULLUP);
  }
  return xTaskCreate(buttons_task, "buttons", 2048, NULL, 2, NULL) == pdPASS;
}

size_t buttons_count() {
  return N_BUTTONS;
}

RemoteCmd buttons_cmd(size_t index) {
  return BUTTONS[index].cmd;
}

SemaphoreHandle_t buttons_semaphore(size_t index) {
  return s_semaphores[index];
}
