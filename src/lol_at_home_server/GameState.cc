#include "GameState.h"
#include <spdlog/spdlog.h>

namespace lol_at_home_server {

GameState::GameState(
    std::unordered_map<EntityId, std::unique_ptr<Entity>> startingEntities)
    : gameState_(std::move(startingEntities)) {}

auto GameState::ProcessActionsAndUpdate(const std::vector<GameAction>& actions,
                                        double deltaTimeMs) -> GameStateDelta {
  GameStateDelta gameStateDelta;

  for (const auto& action : actions) {
    if (!gameState_.contains(action.Id)) {
      spdlog::error("Received an action for a non-existant entity");
      return {};
    }

    switch (action.Type) {
      case GameAction::Type::Move:
        gameState_.at(action.Id)->GetStatsRef().EndPosition =
            action.EndPosition;
        break;
        // todo everything else
    }

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
  entity->GetStatsRef().Id = newId;
  gameState_[newId] = std::move(entity);
}

}  // namespace lol_at_home_server
