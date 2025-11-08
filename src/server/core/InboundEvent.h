#pragma once

#include <enet/enet.h>
#include "GameAction.h"

namespace lol_at_home_server {

struct ClientConnectedEvent {};
struct ClientDisconnectedEvent {};

using InboundEventVariant = std::variant<ClientConnectedEvent,
                                         ClientDisconnectedEvent,
                                         lol_at_home_shared::GameActionVariant>;

struct InboundEvent {
  ENetPeer* peer{};
  InboundEventVariant action;
};

}  // namespace lol_at_home_server
