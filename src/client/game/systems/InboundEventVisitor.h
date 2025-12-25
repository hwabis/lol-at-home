#pragma once

#include <entt/entt.hpp>
#include <unordered_map>
#include "InboundEvent.h"

namespace lol_at_home_game {

class InboundEventVisitor {
 public:
  explicit InboundEventVisitor(entt::registry* registry);

  void operator()(const PlayerAssignedEvent& event);
  void operator()(const ChatMessageEvent& event);
  void operator()(const EntityUpdatedEvent& event);
  void operator()(const EntityDeletedEvent& event);

 private:
  entt::registry* registry_;

  // entt::create on different machines does not give deterministic IDs
  std::unordered_map<uint32_t, entt::entity> serverToClient_;
};

}  // namespace lol_at_home_game
