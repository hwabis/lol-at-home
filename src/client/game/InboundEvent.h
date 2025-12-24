#pragma once

#include "domain/ChampionId.h"
#include "domain/GameAction.h"

namespace lol_at_home_game {

struct ChatMessageEvent {
  std::string text;
};

struct ChampionSelectEvent {
  lol_at_home_shared::ChampionId championId;
  lol_at_home_shared::Team::Color teamColor;
};

using InboundEventVariant = std::variant<lol_at_home_shared::GameActionVariant,
                                         ChatMessageEvent,
                                         ChampionSelectEvent>;

struct InboundEvent {
  InboundEventVariant event;
};

}  // namespace lol_at_home_game
