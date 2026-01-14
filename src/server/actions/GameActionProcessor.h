#pragma once

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "abilities/AbilityImpl.h"
#include "domain/EcsComponents.h"
#include "domain/GameAction.h"

namespace lol_at_home_server {

class GameActionProcessor {
 public:
  GameActionProcessor(entt::registry* registry,
                      std::vector<entt::entity>* instantDirty)
      : registry_(registry), instantDirty_(instantDirty) {}

  void operator()(const lol_at_home_shared::MoveAction& action) {
    if (!registry_->valid(action.source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    if (auto* movable =
            registry_->try_get<lol_at_home_shared::Movable>(action.source)) {
      *movable = {.speed = movable->speed,
                  .state = lol_at_home_shared::MovementState::Moving,
                  .targetX = action.targetX,
                  .targetY = action.targetY};
      instantDirty_->push_back(action.source);
    } else {
      spdlog::warn("Entity has no Movable component");
    }
  }

  void operator()(const lol_at_home_shared::AbilityAction& action) {
    if (!registry_->valid(action.source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    auto* abilities =
        registry_->try_get<lol_at_home_shared::Abilities>(action.source);
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

  void operator()(const lol_at_home_shared::AutoAttackAction& /*action*/) {
    // todo
  }

  void operator()(const lol_at_home_shared::StopGameAction& /*action*/) {
    // todo
  }

 private:
  entt::registry* registry_;
  std::vector<entt::entity>* instantDirty_;

  // todo maybe make this into ability registry class
  static auto getAbilityImpl(lol_at_home_shared::AbilityTag abilityId)
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

}  // namespace lol_at_home_server
