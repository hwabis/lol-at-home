#include "InboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "GameStateSerializer.h"
#include "NetworkChannels.h"
#include "PlayerAssignment.h"
#include "PlayerAssignmentSerializer.h"
#include "actions/GameActionProcessor.h"

namespace lol_at_home_server {

InboundEventVisitor::InboundEventVisitor(
    ENetPeer* peer,
    entt::registry* registry,
    std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap,
    ThreadSafeQueue<OutboundPacket>* outbound)
    : peer_(peer),
      registry_(registry),
      peerToEntityMap_(peerToEntityMap),
      outbound_(outbound) {}

void InboundEventVisitor::operator()(const ClientConnectedEvent& event) const {
  entt::entity entity = registry_->create();
  registry_->emplace<lol_at_home_shared::Position>(entity, 100.0, 200.0);
  registry_->emplace<lol_at_home_shared::Health>(entity, 100.0, 100.0, 5.0);
  registry_->emplace<lol_at_home_shared::Movable>(entity, 300.0);

  peerToEntityMap_->emplace(peer_, entity);

  lol_at_home_shared::PlayerAssignment assignment{entity};
  auto assignmentBytes =
      lol_at_home_shared::PlayerAssignmentSerializer::Serialize(assignment);

  outbound_->Push(
      OutboundPacket{.data = assignmentBytes,
                     .peer = peer_,
                     .channel = lol_at_home_shared::NetworkChannels::Control,
                     .flags = ENET_PACKET_FLAG_RELIABLE});

  auto stateBytes =
      lol_at_home_shared::GameStateSerializer::Serialize(*registry_, {});

  outbound_->Push(
      OutboundPacket{.data = stateBytes,
                     .peer = peer_,
                     .channel = lol_at_home_shared::NetworkChannels::GameState,
                     .flags = ENET_PACKET_FLAG_RELIABLE});
}

void InboundEventVisitor::operator()(
    const ClientDisconnectedEvent& /*event*/) const {
  auto iterator = peerToEntityMap_->find(peer_);
  if (iterator != peerToEntityMap_->end()) {
    registry_->destroy(iterator->second);
    peerToEntityMap_->erase(iterator);
  }
}

void InboundEventVisitor::operator()(const InboundChatEvent& event) const {
  spdlog::info("Client  sent chat: " + event.message);
  // todo broadcast
}

void InboundEventVisitor::operator()(
    const lol_at_home_shared::GameActionVariant& action) const {
  std::visit(GameActionProcessor{*registry_}, action);
}

}  // namespace lol_at_home_server
