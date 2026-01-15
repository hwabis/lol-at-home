#pragma once

#include <enet/enet.h>
#include <cstddef>
#include <vector>

namespace lah::server {

struct OutboundEvent {
  ENetPeer* target = nullptr;  // nullptr = broadcast
  std::vector<std::byte> s2cMessage;
};

}  // namespace lah::server
