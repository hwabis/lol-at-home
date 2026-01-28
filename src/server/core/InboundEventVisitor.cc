#include "InboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "actions/GameActionProcessor.h"
#include "champions/ChampionFactory.h"
#include "serialization/S2CMessageSerializer.h"

namespace lah::server {

InboundEventVisitor::InboundEventVisitor(
    ENetPeer* peer,
    entt::registry* registry,
    std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap,
    std::vector<entt::entity>* instantDirty,
    std::vector<entt::entity>* deletedEntities,
    ThreadSafeQueue<OutboundEvent>* outbound)
    : peer_(peer),
      registry_(registry),
      peerToEntityMap_(peerToEntityMap),
      instantDirty_(instantDirty),
      deletedEntities_(deletedEntities),
      outbound_(outbound) {}

void InboundEventVisitor::operator()(const ChampionSelectedEvent& event) const {
  auto entity = ChampionFactory::CreateChampion(*registry_, event.championId,
                                                event.teamColor);

  peerToEntityMap_->emplace(peer_, entity);

  auto payload = lah::shared::S2CMessageSerializer::SerializePlayerAssignment(
      static_cast<uint32_t>(entity));
  outbound_->Push(OutboundEvent{.target = peer_, .s2cMessage = payload});

  // Send full game state to new player
  std::vector<entt::entity> allEntities;
  for (auto entity : registry_->view<entt::entity>()) {
    allEntities.push_back(entity);
  }
  auto gameStatePayload =
      lah::shared::S2CMessageSerializer::SerializeGameStateDelta(
          *registry_, allEntities, {});
  outbound_->Push(
      OutboundEvent{.target = peer_, .s2cMessage = gameStatePayload});
}

void InboundEventVisitor::operator()(
    const ClientDisconnectedEvent& /*event*/) const {
  auto iterator = peerToEntityMap_->find(peer_);
  if (iterator != peerToEntityMap_->end()) {
    deletedEntities_->push_back(iterator->second);
    registry_->destroy(iterator->second);
    peerToEntityMap_->erase(iterator);
  }
}

void InboundEventVisitor::operator()(const InboundChatEvent& event) const {
  spdlog::info("Client sent chat: " + event.message);

  auto itr = peerToEntityMap_->find(peer_);
  if (itr == peerToEntityMap_->end()) {
    return;
  }

  entt::entity senderEntity = itr->second;
  auto payload = lah::shared::S2CMessageSerializer::SerializeChatBroadcast(
      static_cast<uint32_t>(senderEntity), event.message);
  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
}

void InboundEventVisitor::operator()(
    const lah::shared::GameActionVariant& action) const {
  std::visit(GameActionProcessor{registry_, instantDirty_}, action);
}

}  // namespace lah::server
