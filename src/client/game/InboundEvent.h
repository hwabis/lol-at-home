#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include "Components.h"
#include "Vector2.h"

namespace lol_at_home_game {

struct PlayerAssignedEvent {
  uint32_t myEntityId{};
};

struct ChatMessageEvent {
  uint32_t senderEntityId{};
  std::string text;
};

struct EntityUpdatedEvent {
  uint32_t serverEntityId{};
  lol_at_home_engine::Vector2 worldPosition{};
  Health health{};
  Team team{};
  // todo add all the other components as well THIS IS SO ANNOYING
};

struct EntityDeletedEvent {
  uint32_t serverEntityId{};
};

using InboundEventVariant = std::variant<PlayerAssignedEvent,
                                         ChatMessageEvent,
                                         EntityUpdatedEvent,
                                         EntityDeletedEvent>;

struct InboundEvent {
  InboundEventVariant event;
};

}  // namespace lol_at_home_game
