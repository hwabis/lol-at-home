#include "GameState.h"

namespace lol_at_home_server {

GameState::GameState(
    std::unordered_map<EntityId, std::unique_ptr<Entity>> startGameState)
    : gameState_(std::move(startGameState)) {}

auto GameState::ProcessInputsAndUpdate(const std::vector<PlayerInput>& inputs,
                                       double deltaTimeMs)
    -> std::vector<GameStateDelta> {
  std::vector<GameStateDelta> gameStateDelta;

  for (const auto& input : inputs) {
    // todo: process(input);
    // Build delta as we update
  }

  for (auto& [entityId, entity] : gameState_) {
    entity->Update(deltaTimeMs);
    // Build delta as we update
  }

  return gameStateDelta;
}

}  // namespace lol_at_home_server
