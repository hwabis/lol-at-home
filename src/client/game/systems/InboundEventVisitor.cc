#include "InboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "Components.h"

namespace lah::game {

InboundEventVisitor::InboundEventVisitor(
    entt::registry* registry,
    std::unordered_map<uint32_t, entt::entity>* serverToClient)
    : registry_(registry), serverToClient_(serverToClient) {}

void InboundEventVisitor::operator()(const PlayerAssignedEvent& event) {
  spdlog::info("We are entity: " + std::to_string(event.myEntityId));

  serverAssignedId_ = event.myEntityId;
}

void InboundEventVisitor::operator()(const ChatMessageEvent& /*event*/) {
  // todo
}

void InboundEventVisitor::operator()(const EntityUpdatedEvent& event) {
  spdlog::debug("Processing inbound event " +
                std::to_string(event.serverEntityId));

  entt::entity clientEntity{};

  if (serverToClient_->contains(event.serverEntityId)) {
    clientEntity = (*serverToClient_)[event.serverEntityId];
  } else {
    clientEntity = registry_->create();
    (*serverToClient_)[event.serverEntityId] = clientEntity;
  }

  if (serverAssignedId_.has_value() &&
      *serverAssignedId_ == event.serverEntityId) {
    if (!registry_->all_of<LocalPlayer>(clientEntity)) {
      registry_->emplace<LocalPlayer>(clientEntity, event.serverEntityId);
      spdlog::info("Tagged local player!");
    }
  }

  if (event.position.has_value()) {
    registry_->emplace_or_replace<lah::shared::Position>(clientEntity,
                                                         *event.position);
  }

  if (event.team.has_value()) {
    registry_->emplace_or_replace<lah::shared::Team>(clientEntity, *event.team);
  }

  if (event.health.has_value()) {
    registry_->emplace_or_replace<lah::shared::Health>(clientEntity,
                                                       *event.health);
  }

  if (event.characterState.has_value()) {
    registry_->emplace_or_replace<lah::shared::CharacterState>(
        clientEntity, *event.characterState);
  }
}

void InboundEventVisitor::operator()(const EntityDeletedEvent& event) {
  if (serverToClient_->contains(event.serverEntityId)) {
    entt::entity clientEntity = (*serverToClient_)[event.serverEntityId];
    registry_->destroy(clientEntity);
    serverToClient_->erase(event.serverEntityId);
  }
}

}  // namespace lah::game
