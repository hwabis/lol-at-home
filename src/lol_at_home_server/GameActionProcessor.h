#pragma once

#include <spdlog/spdlog.h>
#include "Entity.h"
#include "EntityStats.h"
#include "GameAction.h"

namespace lol_at_home_server {

class GameActionProcessor {
 public:
  explicit GameActionProcessor(GameStateEntities& gameState)
      : gameState_(gameState) {}

  void operator()(const MoveAction& action) {
    if (auto itr = gameState_.find(action.Id); itr != gameState_.end()) {
      std::visit(
          [&](auto& stats) {
            if constexpr (requires { stats.EndPosition; }) {
              stats.EndPosition = action.EndPosition;
            } else {
              spdlog::warn(
                  "Attempted to process a game action on an incompatible "
                  "entity");
            }
          },
          itr->second->GetStatsRef());
    }
  }

  void operator()(const AbilityGameAction& action) {
    // TODO: Implement ability logic
    if (auto itr = gameState_.find(action.Id); itr != gameState_.end()) {
      // Handle ability cast
    }
  }

  void operator()(const AutoAttackGameAction& action) {
    // TODO: Implement auto-attack logic
    if (auto itr = gameState_.find(action.Id); itr != gameState_.end()) {
      // Handle auto-attack
    }
  }

  void operator()(const StopGameAction& action) {
    if (auto itr = gameState_.find(action.Id); itr != gameState_.end()) {
      std::visit(
          [&](auto& stats) {
            if constexpr (requires {
                            stats.EndPosition;
                            stats.Position;
                          }) {
              stats.EndPosition = stats.Position;
            } else {
              spdlog::warn(
                  "Attempted to process a game action on an incompatible "
                  "entity");
            }
          },
          itr->second->GetStatsRef());
    }
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  GameStateEntities& gameState_;
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
