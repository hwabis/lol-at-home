#pragma once

#include <enet/enet.h>
#include "domain/ChampionId.h"
#include "domain/GameAction.h"

namespace lol_at_home_server {

struct ChampionSelectedEvent {
  lol_at_home_shared::ChampionId championId;
  lol_at_home_shared::Team::Color teamColor;
};
struct ClientDisconnectedEvent {};
struct InboundChatEvent {
  std::string message;
};

using InboundEventVariant = std::variant<ChampionSelectedEvent,
                                         ClientDisconnectedEvent,
                                         InboundChatEvent,
                                         lol_at_home_shared::GameActionVariant>;

struct InboundEvent {
  ENetPeer* peer{};
  InboundEventVariant event;
};

}  // namespace lol_at_home_server
