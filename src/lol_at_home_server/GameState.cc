#include "GameState.h"

namespace lol_at_home_server {

GameState::GameState(
    std::unordered_map<EntityId, std::unique_ptr<Entity>> startGameState)
    : gameState_(std::move(startGameState)) {}

void GameState::Update(double deltaTimeMs) {
  gameStateDelta_.clear();

  for (auto& [entityId, entity] : gameState_) {
    entity->Update(deltaTimeMs);
    // todo update gameStateDelta_ somehow
  }
}

void GameState::Process(PlayerInput input) {
  switch (input.Type) {
    // todo
  }
}

}  // namespace lol_at_home_server
