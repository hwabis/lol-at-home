#include "GameState.h"
#include <spdlog/spdlog.h>
#include "GameActionProcessor.h"

namespace lol_at_home_server {

GameState::GameState(GameStateEntities startingEntities)
    : gameState_(std::move(startingEntities)) {}

auto GameState::ProcessActionsAndUpdate(
    const std::vector<GameActionVariant>& actions,
    double deltaTimeMs) -> GameStateDelta {
  GameStateDelta gameStateDelta;

  for (const auto& action : actions) {
    EntityId actionEntityId = GetEntityId(action);

    if (!gameState_.contains(actionEntityId)) {
      spdlog::error("Received an action for a non-existant entity");
      return {};
    }

    std::visit(GameActionProcessor{gameState_}, action);

    gameStateDelta.Actions.push_back(action);
  }

  for (auto& [entityId, entity] : gameState_) {
    bool entityChanged = entity->Update(deltaTimeMs);

    if (entityChanged) {
      gameStateDelta.UpdatedEntities.push_back(entity->GetStatsRef());
    }
  }

  return gameStateDelta;
}

void GameState::AddEntity(std::unique_ptr<Entity> entity) {
  EntityId newId = ++nextEntityId_;
  std::visit([newId](auto& stats) { stats.Id = newId; }, entity->GetStatsRef());
  gameState_[newId] = std::move(entity);
}

}  // namespace lol_at_home_server
