#pragma once

#include <enet/enet.h>
#include "GameAction.h"

namespace lol_at_home_server {

struct ClientConnectedEvent {};
struct ClientDisconnectedEvent {};
struct InboundChatEvent {
  std::string message;
};

using InboundEventVariant = std::variant<ClientConnectedEvent,
                                         ClientDisconnectedEvent,
                                         InboundChatEvent,
                                         lol_at_home_shared::GameActionVariant>;

struct InboundEvent {
  ENetPeer* peer{};
  InboundEventVariant action;  // todo rename to event
};

}  // namespace lol_at_home_server
