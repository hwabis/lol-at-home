#include "GameState.h"
#include <spdlog/spdlog.h>
#include "GameActionProcessor.h"

namespace lol_at_home_server {

auto GameState::ProcessActionsAndUpdate(
    const std::vector<GameActionVariant>& actions,
    double deltaTimeMs) -> GameStateDelta {
  GameStateDelta gameStateDelta;

  for (const auto& action : actions) {
    std::visit(GameActionProcessor{Registry}, action);
    gameStateDelta.Actions.push_back(action);
  }

  updateMovementSystem(deltaTimeMs, gameStateDelta.ChangedEntities);
  updateHealthSystem(deltaTimeMs, gameStateDelta.ChangedEntities);

  return gameStateDelta;
}

void GameState::updateMovementSystem(double deltaTimeMs,
                                     std::vector<entt::entity>& dirtyEntities) {
  auto view = Registry.view<Position, LinearMovement>();

  for (auto entity : view) {
    auto& pos = view.get<Position>(entity);
    auto& movement = view.get<LinearMovement>(entity);

    double deltaX = movement.TargetPosition.X - pos.X;
    double deltaY = movement.TargetPosition.Y - pos.Y;
    double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

    if (distance < 1.0) {
      Registry.remove<LinearMovement>(entity);
      pos = movement.TargetPosition;
    } else {
      double moveDistance = movement.Speed * (deltaTimeMs / 1000.0);
      double ratio = std::min(moveDistance / distance, 1.0);

      pos.X += deltaX * ratio;
      pos.Y += deltaY * ratio;
    }

    dirtyEntities.push_back(entity);
  }
}

void GameState::updateHealthSystem(double deltaTimeMs,
                                   std::vector<entt::entity>& dirtyEntities) {
  auto view = Registry.view<Health>();

  for (auto entity : view) {
    auto& health = view.get<Health>(entity);

    if (health.CurrentHealth < health.MaxHealth) {
      constexpr double msInSec = 1000.0;
      health.CurrentHealth = std::min(
          health.MaxHealth, health.CurrentHealth + ((deltaTimeMs / msInSec) *
                                                    health.HealthRegenPerSec));
    }

    dirtyEntities.push_back(entity);
  }
}

}  // namespace lol_at_home_server
