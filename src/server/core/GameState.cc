#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include "core/InboundEventVisitor.h"
#include "ecs/AutoAttackSystem.h"
#include "ecs/HealthSystem.h"
#include "ecs/MovementSystem.h"
#include "serialization/S2CMessageSerializer.h"

namespace lah::server {

GameState::GameState(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                     std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound,
                     int simulationHz)
    : inbound_(std::move(inbound)),
      outbound_(std::move(outbound)),
      simulationHz_(simulationHz) {
  // Unfortunately order of systems kinda matters here
  systems_.push_back(std::make_unique<MovementSystem>());
  systems_.push_back(std::make_unique<AutoAttackSystem>());
  systems_.push_back(std::make_unique<HealthSystem>());

  for (auto& system : systems_) {
    int syncHz = system->GetPeriodicSyncRateHz();
    if (syncHz > simulationHz_) {
      spdlog::warn("System wants " + std::to_string(syncHz) +
                   "Hz but simulation is " + std::to_string(simulationHz_) +
                   "Hz. Clamping");
      syncHz = simulationHz_;
    }
    syncIntervals_[system.get()] = simulationHz_ / syncHz;
  }
}

auto GameState::Cycle(std::chrono::milliseconds timeElapsed) -> void {
  ++tickCounter_;

  std::vector<entt::entity> instantDirty;
  std::vector<entt::entity> inboundDeleted;
  processInbound(instantDirty, inboundDeleted);
  auto [dirtyEntities, deletedEntities] = cycleSystems(timeElapsed);

  dirtyEntities.insert(dirtyEntities.end(), instantDirty.begin(),
                       instantDirty.end());
  deletedEntities.insert(deletedEntities.end(), inboundDeleted.begin(),
                         inboundDeleted.end());

  pushOutbound(dirtyEntities, deletedEntities);
}

void GameState::processInbound(std::vector<entt::entity>& instantDirty,
                               std::vector<entt::entity>& deletedEntities) {
  std::queue<InboundEvent> inboundEvents = inbound_->PopAll();

  while (!inboundEvents.empty()) {
    InboundEvent event = inboundEvents.front();
    inboundEvents.pop();

    if (std::holds_alternative<lah::shared::GameActionVariant>(event.event)) {
      if (!validateInboundEventPeer(event)) {
        continue;
      }
    }

    std::visit(
        InboundEventVisitor{event.peer, &registry_, &peerToEntityMap_,
                            &instantDirty, &deletedEntities, outbound_.get()},
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
        std::get<lah::shared::GameActionVariant>(event.event));

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

auto GameState::cycleSystems(std::chrono::milliseconds timeElapsed)
    -> std::pair<std::vector<entt::entity>, std::vector<entt::entity>> {
  // todo apparently u can mark dirty with entt::registry::patch or something ??
  std::vector<entt::entity> periodicDirty;
  std::vector<entt::entity> instantDirty;
  std::vector<entt::entity> deletedEntities;
  for (auto& system : systems_) {
    std::vector<entt::entity> systemPeriodic;
    std::vector<entt::entity> systemInstant;

    system->Cycle(registry_, timeElapsed, systemPeriodic, systemInstant,
                  deletedEntities);

    instantDirty.insert(instantDirty.end(), systemInstant.begin(),
                        systemInstant.end());

    if (tickCounter_ % syncIntervals_[system.get()] == 0) {
      periodicDirty.insert(periodicDirty.end(), systemPeriodic.begin(),
                           systemPeriodic.end());
    }
  }

  std::vector<entt::entity> allDirty = instantDirty;
  allDirty.insert(allDirty.end(), periodicDirty.begin(), periodicDirty.end());

  return {allDirty, deletedEntities};
}

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities,
                             const std::vector<entt::entity>& deletedEntities) {
  auto payload = lah::shared::S2CMessageSerializer::SerializeGameStateDelta(
      registry_, dirtyEntities, deletedEntities);

  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
}

}  // namespace lah::server
