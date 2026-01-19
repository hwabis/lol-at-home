#include "InboundEventVisitor.h"
#include <spdlog/spdlog.h>
#include "Components.h"

namespace lah::game {

InboundEventVisitor::InboundEventVisitor(entt::registry* registry)
    : registry_(registry) {}

auto InboundEventVisitor::findClientEntityByServerId(uint32_t serverId)
    -> std::optional<entt::entity> {
  auto view = registry_->view<ServerEntityId>();
  for (auto entity : view) {
    if (view.get<ServerEntityId>(entity).id == serverId) {
      return entity;
    }
  }
  return std::nullopt;
}

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

  auto existing = findClientEntityByServerId(event.serverEntityId);
  if (existing.has_value()) {
    clientEntity = *existing;
  } else {
    clientEntity = registry_->create();
    registry_->emplace<ServerEntityId>(clientEntity, event.serverEntityId);
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
  auto existing = findClientEntityByServerId(event.serverEntityId);
  if (existing.has_value()) {
    registry_->destroy(*existing);
  }
}

}  // namespace lah::game
