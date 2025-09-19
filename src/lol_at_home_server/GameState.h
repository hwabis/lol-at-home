#pragma once

#include <memory>
#include <unordered_map>
#include "Entity.h"
#include "PlayerInput.h"

namespace lol_at_home_server {

struct GameStateDelta {
  // todo ??????
};

class GameState {
 public:
  explicit GameState(
      std::unordered_map<EntityId, std::unique_ptr<Entity>> startingEntities);
  void Update(double deltaTimeMs);
  void Process(PlayerInput input);
  [[nodiscard]] auto GetFullGameState() const
      -> const std::unordered_map<EntityId, std::unique_ptr<Entity>>& {
    return gameState_;
  }
  [[nodiscard]] auto GetDeltaSincePrevUpdate() const
      -> const std::vector<GameStateDelta>& {
    return gameStateDelta_;
  }

 private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> gameState_;
  std::vector<GameStateDelta> gameStateDelta_;
};

}  // namespace lol_at_home_server
