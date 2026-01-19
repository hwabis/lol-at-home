#pragma once

#include <entt/entt.hpp>
#include <optional>
#include "InboundEvent.h"

namespace lah::game {

class InboundEventVisitor {
 public:
  explicit InboundEventVisitor(entt::registry* registry);

  void operator()(const PlayerAssignedEvent& event);
  void operator()(const ChatMessageEvent& event);
  void operator()(const EntityUpdatedEvent& event);
  void operator()(const EntityDeletedEvent& event);

 private:
  auto findClientEntityByServerId(uint32_t serverId)
      -> std::optional<entt::entity>;

  entt::registry* registry_;
  inline static std::optional<uint32_t> serverAssignedId_;
};

}  // namespace lah::game
