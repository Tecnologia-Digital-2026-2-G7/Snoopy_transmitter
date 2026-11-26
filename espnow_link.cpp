#include "espnow_link.h"
#include <WiFi.h>
#include <esp_now.h>

struct SenderContext {
  RemoteCmd cmd;
  SemaphoreHandle_t semaphore;
};

static const size_t MAX_SENDERS = 8;
static SenderContext s_contexts[MAX_SENDERS];
static size_t s_contextCount = 0;
static uint8_t s_peer[6];
static uint8_t s_seq = 0;

static void sender_task(void *arg) {
  SenderContext *ctx = (SenderContext *)arg;
  for (;;) {
    if (xSemaphoreTake(ctx->semaphore, portMAX_DELAY) == pdTRUE) {
      RemotePacket pkt = { ctx->cmd, __atomic_fetch_add(&s_seq, 1, __ATOMIC_RELAXED) };
      esp_err_t err = esp_now_send(s_peer, (const uint8_t *)&pkt, sizeof(pkt));
      if (err != ESP_OK) {
        Serial.printf("esp_now_send error: %d\n", err);
      }
    }
  }
}

bool espnow_link_begin(const uint8_t peerMac[6]) {
  memcpy(s_peer, peerMac, 6);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return false;
  }

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, s_peer, 6);
  peer.channel = 0;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Error agregando peer");
    return false;
  }
  return true;
}

bool espnow_link_attach(RemoteCmd cmd, SemaphoreHandle_t semaphore) {
  if (s_contextCount >= MAX_SENDERS) return false;

  s_contexts[s_contextCount] = { cmd, semaphore };

  char name[16];
  snprintf(name, sizeof(name), "espnow_tx%u", (unsigned)cmd);
  BaseType_t created = xTaskCreate(sender_task, name, 3072, &s_contexts[s_contextCount], 3, NULL);
  if (created != pdPASS) return false;

  s_contextCount++;
  return true;
}
