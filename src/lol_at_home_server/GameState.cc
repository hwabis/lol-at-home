#include "GameState.h"

namespace lol_at_home_server {

void GameState::Tick(double deltaTimeMs) {
  for (auto& [entityId, entity] : entities) {
    // todo
    // entity->Update(deltaTime);
  }
}

void GameState::Process(PlayerInput input) {
  switch (input.Type) {
    // todo
  }
}

}  // namespace lol_at_home_server
