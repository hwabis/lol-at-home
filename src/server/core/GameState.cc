#include "core/GameState.h"
#include <spdlog/spdlog.h>
#include "core/InboundEventVisitor.h"
#include "ecs/HealthSystem.h"
#include "ecs/MovementSystem.h"
#include "s2c_message_generated.h"
#include "serialization/GameStateSerializer.h"

namespace lol_at_home_server {

GameState::GameState(std::shared_ptr<ThreadSafeQueue<InboundEvent>> inbound,
                     std::shared_ptr<ThreadSafeQueue<OutboundEvent>> outbound,
                     int simulationHz)
    : inbound_(std::move(inbound)),
      outbound_(std::move(outbound)),
      simulationHz_(simulationHz) {
  // Unfortunately order of systems kinda matters here. e.g. HealthSystem needs
  // to see some components that are attached by previous systems
  // todo combat/damage system needs to go before health system
  systems_.push_back(std::make_unique<MovementSystem>());
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

  processInbound();

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

  pushOutbound(allDirty, deletedEntities);
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

void GameState::pushOutbound(const std::vector<entt::entity>& dirtyEntities,
                             const std::vector<entt::entity>& deletedEntities) {
  flatbuffers::FlatBufferBuilder builder(1024);
  auto snapshotOffset = lol_at_home_shared::GameStateSerializer::Serialize(
      builder, registry_, dirtyEntities, deletedEntities);
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      builder, lol_at_home_shared::S2CDataFB::GameStateDeltaFB,
      snapshotOffset.Union());
  builder.Finish(s2cMessage);

  std::vector<std::byte> payload(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));

  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
}

}  // namespace lol_at_home_server
