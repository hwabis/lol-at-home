#pragma once

#include <enet/enet.h>
#include <vector>

namespace lol_at_home_server {

struct OutboundPacket {
  // todo turn this into OutboundEventVariant if server ever needs to send
  // stuff other than game state (e.g. chat)
  std::vector<std::byte> data;
  ENetPeer* peer = nullptr;  // nullptr = broadcast // todo rename to target
  uint8_t channel;           // todo enum-ify
  ENetPacketFlag flags;
};

}  // namespace lol_at_home_server
