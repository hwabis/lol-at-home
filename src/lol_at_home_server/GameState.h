#pragma once

#include <memory>
#include <unordered_map>
#include "Entity.h"
#include "GameAction.h"
#include "GameStateDelta.h"

namespace lol_at_home_server {

class GameState {
 public:
  explicit GameState(
      std::unordered_map<EntityId, std::unique_ptr<Entity>> startingEntities);
  auto ProcessActionsAndUpdate(const std::vector<GameAction>& actions,
                               double deltaTimeMs) -> GameStateDelta;
  [[nodiscard]] auto GetFullGameState() const
      -> const std::unordered_map<EntityId, std::unique_ptr<Entity>>& {
    return gameState_;
  }

 private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> gameState_;
};

}  // namespace lol_at_home_server
