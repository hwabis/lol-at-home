#include "OutboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "GameStateSerializer.h"

namespace lol_at_home_server {

OutboundEventVisitor::OutboundEventVisitor(
    const entt::registry* registry,
    flatbuffers::FlatBufferBuilder* builder)
    : registry_(registry), builder_(builder) {}

void OutboundEventVisitor::operator()(const SendGameStateEvent& event) const {
  // 1. Serialize Game State to FlatBuffers offset
  auto snapshot_offset = lol_at_home_shared::GameStateSerializer::Serialize(
      builder_, registry_, event.dirtyEntities);

  // 2. Build the S2C envelope
  auto s2c_message = lol_at_home_shared::CreateS2CMessageFB(
      builder_, lol_at_home_shared::S2CDataFB::GameStateSnapshot,
      snapshot_offset.Union());

  // Finalize the buffer for sending
  builder_->Finish(s2c_message);
}

void OutboundEventVisitor::operator()(
    const SendPlayerAssignmentEvent& event) const {
  // 1. Build the PlayerAssignment FlatBuffers object
  auto pa_offset = lol_at_home_shared::CreatePlayerAssignmentFB(
      builder_, static_cast<uint32_t>(event.assignment.AssignedEntity));

  // 2. Build the S2C envelope
  auto s2c_message = lol_at_home_shared::CreateS2CMessageFB(
      builder_, lol_at_home_shared::S2CDataFB::PlayerAssignmentFB,
      pa_offset.Union());
  builder_->Finish(s2c_message);
}

void OutboundEventVisitor::operator()(const BroadcastChatEvent& event) const {
  // 1. Build the ChatBroadcast FlatBuffers object
  auto text_offset = builder_->CreateString(event.message);
  auto chat_offset = lol_at_home_shared::CreateChatBroadcastFB(
      builder_, static_cast<uint32_t>(event.sender), text_offset);

  // 2. Build the S2C envelope
  auto s2c_message = lol_at_home_shared::CreateS2CMessageFB(
      builder_, lol_at_home_shared::S2CDataFB::ChatBroadcastFB,
      chat_offset.Union());
  builder_->Finish(s2c_message);
}

}  // namespace lol_at_home_server
