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

  // todo we can replace with a proper struct if we want lol
  registry_->ctx().emplace<uint32_t>(event.myEntityId);
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

    registry_->emplace<Transform>(clientEntity);
    registry_->emplace<RenderableCircle>(clientEntity, 50.0F);
  }

  auto* pendingId = registry_->ctx().find<uint32_t>();
  if ((pendingId != nullptr) && event.serverEntityId == *pendingId) {
    if (!registry_->all_of<LocalPlayer>(clientEntity)) {
      registry_->emplace<LocalPlayer>(clientEntity, event.serverEntityId);
      spdlog::info("Tagged local player!");
    }
  }

  auto& transform = registry_->get<Transform>(clientEntity);
  transform.worldPosition = event.worldPosition;
}

void InboundEventVisitor::operator()(const EntityDeletedEvent& event) {
  if (serverToClient_->contains(event.serverEntityId)) {
    entt::entity clientEntity = (*serverToClient_)[event.serverEntityId];
    registry_->destroy(clientEntity);
    serverToClient_->erase(event.serverEntityId);
  }
}

}  // namespace lol_at_home_game
