#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include "serialization/S2CMessageSerializer.h"

namespace lah::game {

struct PlayerAssignedEvent {
  uint32_t myEntityId{};
};

struct ChatMessageEvent {
  uint32_t senderEntityId{};
  std::string text;
};

// Use the shared EntitySnapshot type, just for serialization/domain separation
using EntityUpdatedEvent = lah::shared::EntitySnapshot;

struct EntityDeletedEvent {
  uint32_t entityId{};
};

using InboundEventVariant = std::variant<PlayerAssignedEvent,
                                         ChatMessageEvent,
                                         EntityUpdatedEvent,
                                         EntityDeletedEvent>;

struct InboundEvent {
  InboundEventVariant event;
};

}  // namespace lah::game
