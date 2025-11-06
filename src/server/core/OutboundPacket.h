#pragma once

#include <enet/enet.h>
#include "GameAction.h"

namespace lol_at_home_server {

struct OutboundPacket {
  std::vector<std::byte> data;
  ENetPeer* peer = nullptr;  // Target. nullptr = broadcast
  uint8_t channel;
  ENetPacketFlag flags;
};

}  // namespace lol_at_home_server
