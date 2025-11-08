#include "OutboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "GameStateSerializer.h"
#include "s2c_message_generated.h"

namespace lol_at_home_server {

OutboundEventVisitor::OutboundEventVisitor(
    const entt::registry* registry,
    flatbuffers::FlatBufferBuilder* builder)
    : registry_(registry), builder_(builder) {}

void OutboundEventVisitor::operator()(const SendGameStateEvent& event) const {
  auto snapshotOffset = lol_at_home_shared::GameStateSerializer::Serialize(
      *builder_, *registry_, event.dirtyEntities);
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      *builder_, lol_at_home_shared::S2CDataFB::GameStateSnapshotFB,
      snapshotOffset.Union());
  builder_->Finish(s2cMessage);
}

void OutboundEventVisitor::operator()(
    const SendPlayerAssignmentEvent& event) const {
  auto paOffset = lol_at_home_shared::CreatePlayerAssignmentFB(
      *builder_, static_cast<uint32_t>(event.assignment.AssignedEntity));
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      *builder_, lol_at_home_shared::S2CDataFB::PlayerAssignmentFB,
      paOffset.Union());
  builder_->Finish(s2cMessage);
}

void OutboundEventVisitor::operator()(const BroadcastChatEvent& event) const {
  auto textOffset = builder_->CreateString(event.message);
  auto chatOffset = lol_at_home_shared::CreateChatBroadcastFB(
      *builder_, static_cast<uint32_t>(event.sender), textOffset);
  auto s2cMessage = lol_at_home_shared::CreateS2CMessageFB(
      *builder_, lol_at_home_shared::S2CDataFB::ChatBroadcastFB,
      chatOffset.Union());
  builder_->Finish(s2cMessage);
}

}  // namespace lol_at_home_server
