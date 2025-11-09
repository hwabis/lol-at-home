#pragma once

#include <enet/enet.h>
#include <cstddef>
#include <vector>

namespace lol_at_home_server {

struct OutboundEvent {
  ENetPeer* target = nullptr;  // nullptr = broadcast
  std::vector<std::byte> s2cMessage;
};

}  // namespace lol_at_home_server
