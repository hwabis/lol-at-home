#include "InboundEventVisitor.h"
#include <flatbuffers/flatbuffers.h>
#include <spdlog/spdlog.h>
#include "actions/GameActionProcessor.h"
#include "champions/ChampionFactory.h"
#include "s2c_message_generated.h"
#include "serialization/GameStateSerializer.h"

namespace lah::server {

InboundEventVisitor::InboundEventVisitor(
    ENetPeer* peer,
    entt::registry* registry,
    std::unordered_map<ENetPeer*, entt::entity>* peerToEntityMap,
    std::vector<entt::entity>* instantDirty,
    ThreadSafeQueue<OutboundEvent>* outbound)
    : peer_(peer),
      registry_(registry),
      peerToEntityMap_(peerToEntityMap),
      instantDirty_(instantDirty),
      outbound_(outbound) {}

void InboundEventVisitor::operator()(const ChampionSelectedEvent& event) const {
  auto entity = ChampionFactory::CreateChampion(*registry_, event.championId,
                                                event.teamColor);

  peerToEntityMap_->emplace(peer_, entity);

  flatbuffers::FlatBufferBuilder builder{};
  auto paOffset = lah_shared::CreatePlayerAssignmentFB(
      builder, static_cast<uint32_t>(entity));
  auto s2cMessage = lah_shared::CreateS2CMessageFB(
      builder, lah_shared::S2CDataFB::PlayerAssignmentFB, paOffset.Union());
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
  auto snapshotOffset = lah::shared::GameStateSerializer::Serialize(
      builder, *registry_, allEntities, {});  // No deletions
  s2cMessage = lah_shared::CreateS2CMessageFB(
      builder, lah_shared::S2CDataFB::GameStateDeltaFB, snapshotOffset.Union());
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
  flatbuffers::FlatBufferBuilder builder{};

  auto textOffset = builder.CreateString(event.message);
  auto chatBroadcast = lah_shared::CreateChatBroadcastFB(
      builder, static_cast<uint32_t>(senderEntity), textOffset);
  auto s2cMessage = lah_shared::CreateS2CMessageFB(
      builder, lah_shared::S2CDataFB::ChatBroadcastFB, chatBroadcast.Union());
  builder.Finish(s2cMessage);

  std::vector<std::byte> payload(
      reinterpret_cast<std::byte*>(builder.GetBufferPointer()),
      reinterpret_cast<std::byte*>(builder.GetBufferPointer() +
                                   builder.GetSize()));
  outbound_->Push(OutboundEvent{.target = nullptr, .s2cMessage = payload});
}

void InboundEventVisitor::operator()(
    const lah::shared::GameActionVariant& action) const {
  std::visit(GameActionProcessor{registry_, instantDirty_}, action);
}

}  // namespace lah::server
