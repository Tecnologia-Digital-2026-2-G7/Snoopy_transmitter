#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "remote_protocol.h"

bool espnow_link_begin(const uint8_t peerMac[6]);
bool espnow_link_attach(RemoteCmd cmd, SemaphoreHandle_t semaphore);
