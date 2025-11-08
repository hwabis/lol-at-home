#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <utility>
#include "core/InboundEventVisitor.h"

namespace lol_at_home_server {

GameState::GameState(std::shared_ptr<ThreadSafeRegistry> registry,
                     std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                     std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound)
    : registry_(std::move(registry)),
      inbound_(std::move(inbound)),
      outbound_(std::move(outbound)) {}

auto GameState::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  // a bit awkward but after getting this lock the rest of cycle can use
  // registry_->GetRegistry(). the other todo should eventually fix this
  auto lock = registry_->GetWriteLock();

  processInbound();

  std::vector<entt::entity> dirtyEntities;
  std::vector<entt::entity> deletedEntities;
  updateSimulation(timeElapsed, dirtyEntities, deletedEntities);

  pushOutbound(dirtyEntities, deletedEntities);
}

void GameState::processInbound() {
  std::queue<InboundEvent> InboundEvents = inbound_->PopAll();

  while (!InboundEvents.empty()) {
    InboundEvent packet = InboundEvents.front();
    InboundEvents.pop();

    std::visit(InboundEventVisitor{packet.peer, &registry_->GetRegistry(),
                                   &peerToEntityMap_, outbound_.get()},
               packet.action);
  }
}

void GameState::updateSimulation(std::chrono::milliseconds timeElapsed,
                                 std::vector<entt::entity>& dirtyEntities,
                                 std::vector<entt::entity>& deletedEntities) {
  updateMovementSystem(timeElapsed, dirtyEntities);
  updateHealthSystem(timeElapsed, dirtyEntities, deletedEntities);
}

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities,
                             const std::vector<entt::entity>& deletedEntities) {
  if (dirtyEntities.empty()) {
    return;
  }

  outbound_->Push(OutboundEvent{
      .target = nullptr,
      .event = SendGameStateEvent{.dirtyEntities = dirtyEntities,
                                  .deletedEntities = deletedEntities}});
}

void GameState::updateMovementSystem(std::chrono::milliseconds timeElapsed,
                                     std::vector<entt::entity>& dirtyEntities) {
  auto view =
      registry_->GetRegistry()
          .view<lol_at_home_shared::Position, lol_at_home_shared::Movable,
                lol_at_home_shared::Moving>();

  for (auto entity : view) {
    auto& pos = view.get<lol_at_home_shared::Position>(entity);
    auto& movable = view.get<lol_at_home_shared::Movable>(entity);
    auto& moving = view.get<lol_at_home_shared::Moving>(entity);

    double deltaX = moving.TargetPosition.X - pos.X;
    double deltaY = moving.TargetPosition.Y - pos.Y;
    double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

    if (distance < 1.0) {
      registry_->GetRegistry().remove<lol_at_home_shared::Moving>(entity);
      pos = moving.TargetPosition;
    } else {
      // todo not sure yet on how the units work
      constexpr double msPerSec = 1000.0;
      double moveDistance = movable.Speed * (timeElapsed.count() / msPerSec);
      double ratio = std::min(moveDistance / distance, 1.0);

      pos.X += deltaX * ratio;
      pos.Y += deltaY * ratio;
    }

    dirtyEntities.push_back(entity);
  }
}

void GameState::updateHealthSystem(std::chrono::milliseconds timeElapsed,
                                   std::vector<entt::entity>& dirtyEntities,
                                   std::vector<entt::entity>& deletedEntities) {
  auto view = registry_->GetRegistry().view<lol_at_home_shared::Health>();

  for (auto entity : view) {
    auto& health = view.get<lol_at_home_shared::Health>(entity);

    if (health.CurrentHealth < health.MaxHealth) {
      constexpr double msInSec = 1000.0;
      health.CurrentHealth = std::min(
          health.MaxHealth,
          health.CurrentHealth +
              ((timeElapsed.count() / msInSec) * health.HealthRegenPerSec));
      dirtyEntities.push_back(entity);
    } else if (health.CurrentHealth <= 0) {
      deletedEntities.push_back(entity);
      registry_->GetRegistry().destroy(entity);
    }
  }
}

}  // namespace lol_at_home_server
