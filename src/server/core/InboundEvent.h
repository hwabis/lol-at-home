#pragma once

#include <enet/enet.h>
#include "domain/ChampionId.h"
#include "domain/GameAction.h"

namespace lol_at_home_server {

struct ClientConnectedEvent {
  lol_at_home_shared::ChampionId championId;
};
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
  InboundEventVariant event;
};

}  // namespace lol_at_home_server
