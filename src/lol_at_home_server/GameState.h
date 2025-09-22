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
      GameStateEntities startingEntities);
  auto ProcessActionsAndUpdate(const std::vector<GameActionVariant>& actions,
                               double deltaTimeMs) -> GameStateDelta;
  void AddEntity(std::unique_ptr<Entity> entity);
  [[nodiscard]] auto GetFullGameState() const
      -> const GameStateEntities& {
    return gameState_;
  }

 private:
  GameStateEntities gameState_;
  EntityId nextEntityId_ = 0;
};

}  // namespace lol_at_home_server
