#pragma once

#include <enet/enet.h>
#include "domain/ChampionId.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"

namespace lah::server {

struct ChampionSelectedEvent {
  lah::shared::ChampionId championId;
  lah::shared::Team::Color teamColor;
};
struct ClientDisconnectedEvent {};
struct InboundChatEvent {
  std::string message;
};

using InboundEventVariant = std::variant<ChampionSelectedEvent,
                                         ClientDisconnectedEvent,
                                         InboundChatEvent,
                                         lah::shared::GameActionVariant>;

struct InboundEvent {
  ENetPeer* peer{};
  InboundEventVariant event;
};

}  // namespace lah::server
