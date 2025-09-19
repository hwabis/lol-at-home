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
  auto ProcessInputsAndUpdate(const std::vector<PlayerInput>& inputs,
                              double deltaTimeMs)
      -> std::vector<GameStateDelta>;
  [[nodiscard]] auto GetFullGameState() const
      -> const std::unordered_map<EntityId, std::unique_ptr<Entity>>& {
    return gameState_;
  }

 private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> gameState_;
};

}  // namespace lol_at_home_server
