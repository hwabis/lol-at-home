#pragma once

#include <memory>
#include <unordered_map>
#include "Entity.h"
#include "EntityStats.h"
#include "GameAction.h"

namespace lol_at_home_server {

class GameActionProcessor {
 public:
  explicit GameActionProcessor(
      std::unordered_map<EntityId, std::unique_ptr<Entity>>& gameState)
      : gameState_(gameState) {}

  void operator()(const MoveAction& action) {
    if (auto it = gameState_.find(action.Id); it != gameState_.end()) {
      it->second->GetStatsRef().EndPosition = action.EndPosition;
    }
  }

  void operator()(const AbilityGameAction& action) {
    // TODO: Implement ability logic
    if (auto it = gameState_.find(action.Id); it != gameState_.end()) {
      // Handle ability cast
    }
  }

  void operator()(const AutoAttackGameAction& action) {
    // TODO: Implement auto-attack logic
    if (auto it = gameState_.find(action.Id); it != gameState_.end()) {
      // Handle auto-attack
    }
  }

  void operator()(const StopGameAction& action) {
    if (auto it = gameState_.find(action.Id); it != gameState_.end()) {
      // Stop current action - maybe clear EndPosition or set it to
      // CurrentPosition
      auto& stats = it->second->GetStatsRef();
      stats.EndPosition = stats.CurrentPosition;
    }
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  std::unordered_map<EntityId, std::unique_ptr<Entity>>& gameState_;
};

struct GetEntityIdVisitor {
  auto operator()(const MoveAction& action) -> EntityId { return action.Id; }
  auto operator()(const AbilityGameAction& action) -> EntityId {
    return action.Id;
  }
  auto operator()(const AutoAttackGameAction& action) -> EntityId {
    return action.Id;
  }
  auto operator()(const StopGameAction& action) -> EntityId {
    return action.Id;
  }
};

inline auto GetEntityId(const GameActionVariant& action) -> EntityId {
  return std::visit(GetEntityIdVisitor{}, action);
}

}  // namespace lol_at_home_server
