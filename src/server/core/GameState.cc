#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include "GameStateSerializer.h"
#include "core/InboundEventVisitor.h"
#include "s2c_message_generated.h"

namespace lol_at_home_server {

GameState::GameState(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                     std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound)
    : inbound_(std::move(inbound)), outbound_(std::move(outbound)) {}

auto GameState::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  processInbound();

  std::vector<entt::entity> dirtyEntities;
  std::vector<entt::entity> deletedEntities;
  updateSimulation(timeElapsed, dirtyEntities, deletedEntities);

  pushOutbound(dirtyEntities, deletedEntities);
}

void GameState::processInbound() {
  std::queue<InboundEvent> inboundEvents = inbound_->PopAll();

  while (!inboundEvents.empty()) {
    InboundEvent event = inboundEvents.front();
    inboundEvents.pop();

    if (std::holds_alternative<lol_at_home_shared::GameActionVariant>(
            event.event)) {
      if (!validateInboundEventPeer(event)) {
        continue;
      }
    }

    std::visit(InboundEventVisitor{event.peer, &registry_, &peerToEntityMap_,
                                   outbound_.get()},
               event.event);
  }
}

auto GameState::validateInboundEventPeer(const InboundEvent& event) -> bool {
  auto itr = peerToEntityMap_.find(event.peer);
  if (itr != peerToEntityMap_.end()) {
    entt::entity authoritativeSource = itr->second;
    entt::entity claimedSource = std::visit(
        [](const auto& specificAction) -> entt::entity {
          return specificAction.source;
        },
        std::get<lol_at_home_shared::GameActionVariant>(event.event));

    if (claimedSource != authoritativeSource) {
      spdlog::warn("IMPERSONATION ATTEMPT: Peer owns entity " +
                   std::to_string(static_cast<uint32_t>(authoritativeSource)) +
                   " but claimed to be entity " +
                   std::to_string(static_cast<uint32_t>(claimedSource)));
      return false;
    }
  } else {
    spdlog::warn("Received action from unassigned peer ???");
    return false;
  }

  return true;
}

void GameState::updateSimulation(std::chrono::milliseconds timeElapsed,
                                 std::vector<entt::entity>& dirtyEntities,
                                 std::vector<entt::entity>& deletedEntities) {
  updateMovementSystem(timeElapsed, dirtyEntities);
  updateHealthSystem(timeElapsed, dirtyEntities, deletedEntities);
}

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities,
                             const std::vector<entt::entity>& deletedEntities) {
  flatbuffers::FlatBufferBuilder builder(1024);
  auto snapshotOffset = lol_at_home_shared::GameStateSerializer::Serialize(
      builder, registry_, dirtyEntities, deletedEntities);
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      builder, lol_at_home_shared::S2CDataFB::GameStateSnapshotFB,
      snapshotOffset.Union());
  builder.Finish(s2cMessage);

  std::vector<std::byte> payload(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));

  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
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

    double deltaX = moving.targetPosition.x - pos.x;
    double deltaY = moving.targetPosition.y - pos.y;
    double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));

    if (distance < 1.0) {
      registry_.remove<lol_at_home_shared::Moving>(entity);
      pos = moving.targetPosition;
    } else {
      // todo not sure yet on how the units work
      constexpr double msPerSec = 1000.0;
      double moveDistance = movable.speed * (timeElapsed.count() / msPerSec);
      double ratio = std::min(moveDistance / distance, 1.0);

      pos.x += deltaX * ratio;
      pos.y += deltaY * ratio;
    }

    dirtyEntities.push_back(entity);
  }
}

void GameState::updateHealthSystem(std::chrono::milliseconds timeElapsed,
                                   std::vector<entt::entity>& dirtyEntities,
                                   std::vector<entt::entity>& deletedEntities) {
  auto view = registry_.view<lol_at_home_shared::Health>();

  for (auto entity : view) {
    auto& health = view.get<lol_at_home_shared::Health>(entity);

    if (health.currentHealth < health.maxHealth) {
      constexpr double msInSec = 1000.0;
      health.currentHealth = std::min(
          health.maxHealth,
          health.currentHealth +
              ((timeElapsed.count() / msInSec) * health.healthRegenPerSec));
      dirtyEntities.push_back(entity);
    } else if (health.currentHealth <= 0) {
      deletedEntities.push_back(entity);
      registry_.destroy(entity);
    }
  }
}

}  // namespace lol_at_home_server
