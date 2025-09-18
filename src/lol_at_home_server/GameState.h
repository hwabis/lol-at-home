#pragma once

#include <memory>
#include <unordered_map>
#include "Entity.h"
#include "PlayerInput.h"

namespace lol_at_home_server {

class GameState {
 public:
  void Tick(double deltaTimeMs);
  void Process(PlayerInput input);
  [[nodiscard]] auto GetEntities() const
      -> const std::unordered_map<EntityId, std::unique_ptr<Entity>>& {
    return entities;
  }

 private:
  std::unordered_map<EntityId, std::unique_ptr<Entity>> entities;
};

}  // namespace lol_at_home_server
