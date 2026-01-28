#pragma once

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "abilities/AbilityImpl.h"
#include "domain/ArenaConfig.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"
#include "ecs/EcsComponents.h"

namespace lah::server {

class GameActionProcessor {
 public:
  GameActionProcessor(entt::registry* registry,
                      std::vector<entt::entity>* instantDirty)
      : registry_(registry), instantDirty_(instantDirty) {}

  void operator()(const lah::shared::MoveAction& action) {
    if (!registry_->valid(action.source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    if (!lah::shared::IsInArena(action.targetX, action.targetY)) {
      spdlog::info("Move action rejected: outside arena bounds : " +
                   std::to_string(action.targetX) + ", " +
                   std::to_string(action.targetY));
      return;
    }

    auto& moveTarget =
        registry_->emplace_or_replace<lah::shared::MoveTarget>(action.source);
    moveTarget = {.targetX = action.targetX, .targetY = action.targetY};

    auto& characterState =
        registry_->get<lah::shared::CharacterState>(action.source);
    characterState.state = lah::shared::CharacterState::State::Moving;

    instantDirty_->push_back(action.source);
  }

  void operator()(const lah::shared::AbilityAction& action) {
    if (!registry_->valid(action.source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    auto* abilities = registry_->try_get<lah::shared::Abilities>(action.source);
    if (abilities == nullptr) {
      spdlog::warn("Entity has no Abilities component");
      return;
    }

    auto itr = abilities->abilities.find(action.slot);
    if (itr == abilities->abilities.end()) {
      return;
    }

    const auto& ability = itr->second;

    if (ability.cooldownRemaining > 0.0F) {
      return;
    }

    auto abilityImpl = getAbilityImpl(ability.tag);
    abilityImpl->Execute(*registry_, ability, action.target);
  }

  void operator()(const lah::shared::AutoAttackAction& action) {
    auto& sourcePos = registry_->get<lah::shared::Position>(action.source);
    auto& targetPos = registry_->get<lah::shared::Position>(action.target);
    auto& sourceRadius = registry_->get<lah::shared::Radius>(action.source);
    auto& targetRadius = registry_->get<lah::shared::Radius>(action.target);
    auto& attackStats =
        registry_->get<lah::shared::AutoAttackStats>(action.source);

    float xDelta = targetPos.x - sourcePos.x;
    float yDelta = targetPos.y - sourcePos.y;

    float distance = std::sqrt((xDelta * xDelta) + (yDelta * yDelta));
    float effectiveDistance =
        distance - sourceRadius.radius - targetRadius.radius;

    registry_->emplace_or_replace<AutoAttackTarget>(
        action.source, AutoAttackTarget{.target = action.target});

    auto& characterState =
        registry_->get<lah::shared::CharacterState>(action.source);

    if (effectiveDistance <= attackStats.range) {
      characterState.state =
          lah::shared::CharacterState::State::AutoAttackWindup;
      registry_->emplace_or_replace<AutoAttackWindupTimer>(
          action.source,
          AutoAttackWindupTimer{.remaining = attackStats.windupDuration});
    } else {
      float proportion = (distance - sourceRadius.radius - targetRadius.radius -
                          attackStats.range) /
                         distance;

      auto& moveTarget =
          registry_->emplace_or_replace<lah::shared::MoveTarget>(action.source);
      moveTarget = {.targetX = sourcePos.x + (xDelta * proportion),
                    .targetY = sourcePos.y + (yDelta * proportion)};

      characterState.state =
          lah::shared::CharacterState::State::AutoAttackMoving;
    }

    instantDirty_->push_back(action.source);
  }

  void operator()(const lah::shared::StopGameAction& /*action*/) {
    // todo
  }

 private:
  entt::registry* registry_;
  std::vector<entt::entity>* instantDirty_;

  // todo maybe make this into ability registry class
  static auto getAbilityImpl(lah::shared::AbilityTag abilityId)
      -> std::unique_ptr<AbilityImpl> {
    // The reason we do to a switch instead of a map is to cover all AbilityTag
    // at compile time (compiler warning otherwise)
    switch (abilityId) {
      // todo
      default:
        break;
    }
    return nullptr;
  }
};

}  // namespace lah::server
