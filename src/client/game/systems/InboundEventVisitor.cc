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
  spdlog::debug("Processing inbound event " + std::to_string(event.entityId));

  entt::entity clientEntity{};

  auto existing = findClientEntityByServerId(event.entityId);
  if (existing.has_value()) {
    clientEntity = *existing;
  } else {
    clientEntity = registry_->create();
    registry_->emplace<ServerEntityId>(clientEntity, event.entityId);
  }

  if (serverAssignedId_.has_value() && *serverAssignedId_ == event.entityId) {
    if (!registry_->all_of<LocalPlayer>(clientEntity)) {
      registry_->emplace<LocalPlayer>(clientEntity, event.entityId);
      spdlog::info("Tagged local player!");
    }
  }

  if (event.position.has_value()) {
    registry_->emplace_or_replace<lah::shared::Position>(clientEntity,
                                                         *event.position);
  }

  if (event.radius.has_value()) {
    registry_->emplace_or_replace<lah::shared::Radius>(clientEntity,
                                                       *event.radius);
  }

  if (event.health.has_value()) {
    registry_->emplace_or_replace<lah::shared::Health>(clientEntity,
                                                       *event.health);
  }

  if (event.mana.has_value()) {
    registry_->emplace_or_replace<lah::shared::Mana>(clientEntity, *event.mana);
  }

  if (event.movementStats.has_value()) {
    registry_->emplace_or_replace<lah::shared::MovementStats>(
        clientEntity, *event.movementStats);
  }

  if (event.characterState.has_value()) {
    auto state = event.characterState->state;
    registry_->emplace_or_replace<lah::shared::CharacterState>(
        clientEntity, *event.characterState);

    if (state == lah::shared::CharacterState::State::Idle) {
      registry_->remove<lah::shared::MoveTarget>(clientEntity);
    }
  }

  if (event.moveTarget.has_value()) {
    registry_->emplace_or_replace<lah::shared::MoveTarget>(clientEntity,
                                                           *event.moveTarget);
  }

  if (event.team.has_value()) {
    registry_->emplace_or_replace<lah::shared::Team>(clientEntity, *event.team);
  }

  if (event.abilities.has_value()) {
    registry_->emplace_or_replace<lah::shared::Abilities>(clientEntity,
                                                          *event.abilities);
  }
}

void InboundEventVisitor::operator()(const EntityDeletedEvent& event) {
  auto existing = findClientEntityByServerId(event.entityId);
  if (existing.has_value()) {
    registry_->destroy(*existing);
  }
}

}  // namespace lah::game
