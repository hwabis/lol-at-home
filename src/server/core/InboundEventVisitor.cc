#include "InboundEventVisitor.h"
#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>
#include "serialization/GameStateSerializer.h"
#include "actions/GameActionProcessor.h"
#include "s2c_message_generated.h"

namespace lol_at_home_server {

InboundEventVisitor::InboundEventVisitor(
    ENetPeer* peer,
    entt::registry* registry,
    std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap,
    ThreadSafeQueue<OutboundEvent>* outbound)
    : peer_(peer),
      registry_(registry),
      peerToEntityMap_(peerToEntityMap),
      outbound_(outbound) {}

void InboundEventVisitor::operator()(
    const ClientConnectedEvent& /*event*/) const {
  entt::entity entity = registry_->create();

  registry_->emplace<lol_at_home_shared::Position>(entity, 100.0, 200.0);
  registry_->emplace<lol_at_home_shared::Health>(entity, 100.0, 100.0, 5.0);
  registry_->emplace<lol_at_home_shared::Movable>(entity, 300.0);

  peerToEntityMap_->emplace(peer_, entity);

  flatbuffers::FlatBufferBuilder builder(1024);
  auto paOffset = lol_at_home_shared::CreatePlayerAssignmentFB(
      builder, static_cast<uint32_t>(entity));
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      builder, lol_at_home_shared::S2CDataFB::PlayerAssignmentFB,
      paOffset.Union());
  builder.Finish(s2cMessage);

  std::vector<std::byte> payload(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));
  outbound_->Push(OutboundEvent{.target = peer_, .s2cMessage = payload});

  // Send full game state to new player

  builder.Clear();

  std::vector<entt::entity> allEntities;
  for (auto entity : registry_->view<entt::entity>()) {
    allEntities.push_back(entity);
  }
  auto snapshotOffset = lol_at_home_shared::GameStateSerializer::Serialize(
      builder, *registry_, allEntities, {});  // No deletions
  s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      builder, lol_at_home_shared::S2CDataFB::GameStateSnapshotFB,
      snapshotOffset.Union());
  builder.Finish(s2cMessage);

  payload = std::vector<std::byte>(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));
  outbound_->Push(OutboundEvent{.target = peer_, .s2cMessage = payload});
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
  spdlog::info("Client sent chat: " + event.message);

  auto itr = peerToEntityMap_->find(peer_);
  if (itr == peerToEntityMap_->end()) {
    return;
  }

  entt::entity senderEntity = itr->second;
  flatbuffers::FlatBufferBuilder builder(256);

  auto textOffset = builder.CreateString(event.message);
  auto chatBroadcast = lol_at_home_shared::CreateChatBroadcastFB(
      builder, static_cast<uint32_t>(senderEntity), textOffset);
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      builder, lol_at_home_shared::S2CDataFB::ChatBroadcastFB,
      chatBroadcast.Union());
  builder.Finish(s2cMessage);

  std::vector<std::byte> payload(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));
  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
}

void InboundEventVisitor::operator()(
    const lol_at_home_shared::GameActionVariant& action) const {
  std::visit(GameActionProcessor{*registry_}, action);
}

}  // namespace lol_at_home_server
