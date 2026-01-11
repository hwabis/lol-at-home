#include "InboundEventVisitor.h"
#include "Components.h"

#include <spdlog/spdlog.h>
namespace lol_at_home_game {

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

    // default-initialize all these, they will be updated anyway
    registry_->emplace<Transform>(clientEntity);
    registry_->emplace<RenderableCircle>(clientEntity);
    registry_->emplace<Health>(clientEntity);
  }

  if (serverAssignedId_.has_value() &&
      *serverAssignedId_ == event.serverEntityId) {
    if (!registry_->all_of<LocalPlayer>(clientEntity)) {
      registry_->emplace<LocalPlayer>(clientEntity, event.serverEntityId);
      spdlog::info("Tagged local player!");
    }
  }

  auto& transform = registry_->get<Transform>(clientEntity);
  transform.worldPosition = event.worldPosition;
  auto& renderableCircle = registry_->get<RenderableCircle>(clientEntity);
  renderableCircle.radius = 50.0F;  // if server event provides radius, use that
  auto& health = registry_->get<Health>(clientEntity);
  health = event.health;
}

void InboundEventVisitor::operator()(const EntityDeletedEvent& event) {
  if (serverToClient_->contains(event.serverEntityId)) {
    entt::entity clientEntity = (*serverToClient_)[event.serverEntityId];
    registry_->destroy(clientEntity);
    serverToClient_->erase(event.serverEntityId);
  }
}

}  // namespace lol_at_home_game
