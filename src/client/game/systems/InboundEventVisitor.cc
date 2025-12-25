#include "InboundEventVisitor.h"
#include "Components.h"

namespace lol_at_home_game {

InboundEventVisitor::InboundEventVisitor(entt::registry* registry)
    : registry_(registry) {}

void InboundEventVisitor::operator()(const PlayerAssignedEvent& event) {
  // todo
}

void InboundEventVisitor::operator()(const ChatMessageEvent& event) {
  // todo
}

void InboundEventVisitor::operator()(const EntityUpdatedEvent& event) {
  entt::entity clientEntity{};

  if (serverToClient_.contains(event.serverEntityId)) {
    clientEntity = serverToClient_[event.serverEntityId];
  } else {
    clientEntity = registry_->create();
    serverToClient_[event.serverEntityId] = clientEntity;

    registry_->emplace<Transform>(clientEntity);
    registry_->emplace<RenderableCircle>(clientEntity, 50.0F);
  }

  auto& transform = registry_->get<Transform>(clientEntity);
  transform.position = event.position;
}

void InboundEventVisitor::operator()(const EntityDeletedEvent& event) {
  if (serverToClient_.contains(event.serverEntityId)) {
    entt::entity clientEntity = serverToClient_[event.serverEntityId];
    registry_->destroy(clientEntity);
    serverToClient_.erase(event.serverEntityId);
  }
}

}  // namespace lol_at_home_game
