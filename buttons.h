#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "remote_protocol.h"

bool buttons_begin();
size_t buttons_count();
RemoteCmd buttons_cmd(size_t index);
SemaphoreHandle_t buttons_semaphore(size_t index);
