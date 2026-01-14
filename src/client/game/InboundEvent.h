#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include "domain/EcsComponents.h"

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
  std::optional<lol_at_home_shared::Position> position;
  std::optional<lol_at_home_shared::Team> team;
  std::optional<lol_at_home_shared::Health> health;
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
