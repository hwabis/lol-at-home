#pragma once

#include <cstdint>
#include <optional>
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
  std::optional<Transform> transform;
  std::optional<Team> team;
  std::optional<Health> health;
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
