#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include "GameStateSerializer.h"
#include "actions/GameActionProcessor.h"

namespace lol_at_home_server {

auto GameState::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  processInbound();

  std::vector<entt::entity> dirtyEntities;
  updateSimulation(timeElapsed, dirtyEntities);

  pushOutbound(dirtyEntities);
}

void GameState::processInbound() {
  std::vector<InboundPacket> inboundPackets = inbound_->PopAll();

  // todo make the visitor
}

void GameState::updateSimulation(std::chrono::milliseconds timeElapsed,
                                 std::vector<entt::entity>& dirtyEntities) {
  updateMovementSystem(timeElapsed, dirtyEntities);
  updateHealthSystem(timeElapsed, dirtyEntities);
}

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities) {

}

void GameState::updateMovementSystem(std::chrono::milliseconds timeElapsed,
                                     std::vector<entt::entity>& dirtyEntities) {
  auto view =
      registry_.view<lol_at_home_shared::Position, lol_at_home_shared::Movable,
                     lol_at_home_shared::Moving>();

  for (auto entity : view) {
    auto& pos = view.get<lol_at_home_shared::Position>(entity);
    auto& movable = view.get<lol_at_home_shared::Movable>(entity);
    auto& moving = view.get<lol_at_home_shared::Moving>(entity);

    double deltaX = moving.TargetPosition.X - pos.X;
    double deltaY = moving.TargetPosition.Y - pos.Y;
    double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

    if (distance < 1.0) {
      registry_.remove<lol_at_home_shared::Moving>(entity);
      pos = moving.TargetPosition;
    } else {
      constexpr int msPerSec = 1000;  // todo not sure yet on how the units work
      double moveDistance = movable.Speed * (timeElapsed / msPerSec);
      double ratio = std::min(moveDistance / distance, 1.0);

      pos.X += deltaX * ratio;
      pos.Y += deltaY * ratio;
    }

    dirtyEntities.push_back(entity);
  }
}

void GameState::updateHealthSystem(std::chrono::milliseconds timeElapsed,
                                   std::vector<entt::entity>& dirtyEntities) {
  auto view = registry_.view<lol_at_home_shared::Health>();

  for (auto entity : view) {
    auto& health = view.get<lol_at_home_shared::Health>(entity);

    if (health.CurrentHealth < health.MaxHealth) {
      constexpr double msInSec = 1000.0;
      health.CurrentHealth = std::min(
          health.MaxHealth, health.CurrentHealth + ((timeElapsed / msInSec) *
                                                    health.HealthRegenPerSec));
      dirtyEntities.push_back(entity);
    }
  }
}

}  // namespace lol_at_home_server
