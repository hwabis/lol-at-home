#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include "domain/EcsComponents.h"

namespace lah::game {

struct PlayerAssignedEvent {
  uint32_t myEntityId{};
};

struct ChatMessageEvent {
  uint32_t senderEntityId{};
  std::string text;
};

struct EntityUpdatedEvent {
  uint32_t serverEntityId{};
  std::optional<lah::shared::Position> position;
  std::optional<lah::shared::Team> team;
  std::optional<lah::shared::Health> health;
  std::optional<lah::shared::CharacterState> characterState;
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

}  // namespace lah::game
