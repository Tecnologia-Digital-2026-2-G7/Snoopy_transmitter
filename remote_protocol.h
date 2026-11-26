#pragma once
#include <stdint.h>

enum RemoteCmd : uint8_t {
  CMD_NEXT = 1,
  CMD_PREVIOUS,
  CMD_VOLUME_UP,
  CMD_VOLUME_DOWN,
  CMD_PLAY_STOP
};

typedef struct __attribute__((packed)) {
  uint8_t cmd;
  uint8_t seq;
} RemotePacket;
