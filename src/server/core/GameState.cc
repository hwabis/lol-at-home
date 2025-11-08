#define NOMINMAX
#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <utility>
#include "GameStateSerializer.h"
#include "NetworkChannels.h"
#include "core/InboundEventVisitor.h"

namespace lol_at_home_server {

GameState::GameState(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                     std::shared_ptr<ThreadSafeQueue<OutboundPacket>> outbound)
    : inbound_(std::move(inbound)), outbound_(std::move(outbound)) {}

auto GameState::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  processInbound();

  std::vector<entt::entity> dirtyEntities;
  updateSimulation(timeElapsed, dirtyEntities);

  pushOutbound(dirtyEntities);
}

void GameState::processInbound() {
  std::queue<InboundEvent> InboundEvents = inbound_->PopAll();

  while (!InboundEvents.empty()) {
    InboundEvent packet = InboundEvents.front();
    InboundEvents.pop();

    std::visit(InboundEventVisitor{packet.peer, &registry_, &peerToEntityMap_,
                                   outbound_.get()},
               packet.action);
  }
}

void GameState::updateSimulation(std::chrono::milliseconds timeElapsed,
                                 std::vector<entt::entity>& dirtyEntities) {
  updateMovementSystem(timeElapsed, dirtyEntities);
  updateHealthSystem(timeElapsed, dirtyEntities);
}

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities) {
  if (dirtyEntities.empty()) {
    return;
  }

  auto stateBytes = lol_at_home_shared::GameStateSerializer::Serialize(
      registry_, dirtyEntities);

  outbound_->Push(
      OutboundPacket{.data = stateBytes,
                     .peer = nullptr,
                     .channel = lol_at_home_shared::NetworkChannels::GameState,
                     .flags = ENET_PACKET_FLAG_RELIABLE});
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
                                   std::vector<entt::entity>& dirtyEntities) {
  auto view = registry_.view<lol_at_home_shared::Health>();

  for (auto entity : view) {
    auto& health = view.get<lol_at_home_shared::Health>(entity);

    if (health.CurrentHealth < health.MaxHealth) {
      constexpr double msInSec = 1000.0;
      health.CurrentHealth = std::min(
          health.MaxHealth,
          health.CurrentHealth +
              ((timeElapsed.count() / msInSec) * health.HealthRegenPerSec));
      dirtyEntities.push_back(entity);
    }
  }
}

}  // namespace lol_at_home_server
