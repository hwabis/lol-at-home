#pragma once

#include <enet/enet.h>
#include <variant>
#include <vector>
#include "PlayerAssignment.h"

namespace lol_at_home_server {

struct SendGameStateEvent {
  std::vector<entt::entity> dirtyEntities;  // if empty, send full state
};

struct SendPlayerAssignmentEvent {
  lol_at_home_shared::PlayerAssignment assignment;
};

struct BroadcastChatEvent {
  entt::entity sender;
  std::string message;
};

// this variant has to match the fbs union right??
using OutboundEventVariant = std::
    variant<SendGameStateEvent, SendPlayerAssignmentEvent, BroadcastChatEvent>;

struct OutboundEvent {
  ENetPeer* target = nullptr;  // nullptr = broadcast
  OutboundEventVariant event;
};

}  // namespace lol_at_home_server
