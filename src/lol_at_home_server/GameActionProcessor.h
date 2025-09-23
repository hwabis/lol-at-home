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
    // todo
  }

  void operator()(const AutoAttackGameAction& action) {
    // todo
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

inline auto GetEntityId(const GameActionVariant& action) -> EntityId {
  return std::visit([](const auto& actionL) { return actionL.Id; }, action);
}

}  // namespace lol_at_home_server
