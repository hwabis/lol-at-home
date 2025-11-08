#pragma once

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "GameAction.h"
#include "abilities/AbilityImpl.h"

namespace lol_at_home_server {

class GameActionProcessor {
 public:
  explicit GameActionProcessor(entt::registry& registry)
      : registry_(registry) {}

  void operator()(const lol_at_home_shared::MoveAction& action) {
    if (!registry_.valid(action.Source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    if (!registry_.all_of<lol_at_home_shared::Movable>(action.Source)) {
      spdlog::warn("Entity has no Movable component");
      return;
    }

    registry_.emplace_or_replace<lol_at_home_shared::Moving>(
        action.Source, action.TargetPosition);
  }

  void operator()(const lol_at_home_shared::AbilityAction& action) {
    if (!registry_.valid(action.Source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    auto* abilities =
        registry_.try_get<lol_at_home_shared::Abilities>(action.Source);
    if (abilities == nullptr) {
      spdlog::warn("Entity has no Abilities component");
      return;
    }

    auto itr = abilities->Abilities.find(action.Slot);
    if (itr == abilities->Abilities.end()) {
      return;
    }

    const auto& ability = itr->second;

    if (ability.CooldownRemaining > 0.0F) {
      return;
    }

    auto abilityImpl = getAbilityImpl(ability.Tag);
    abilityImpl->Execute(registry_, ability, action.Target);
  }

  void operator()(const lol_at_home_shared::AutoAttackAction& action) {
    // todo
  }

  void operator()(const lol_at_home_shared::StopGameAction& action) {
    // todo
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  // todo pass as ptr like i do for all other visitors and remove ^
  entt::registry& registry_;

  // todo maybe make this into ability registry class
  static auto getAbilityImpl(lol_at_home_shared::AbilityTag abilityId)
      -> std::unique_ptr<AbilityImpl> {
    // The reason we do to a switch instead of a map is to cover all AbilityTag
    // at compile time (compiler warning otherwise)
    switch (abilityId) {
      // todo
      // in fact the compiler is emitting a warning right nao
    }

    throw std::runtime_error("Unhandled AbilityTag: " +
                             std::to_string(static_cast<int>(abilityId)));
  }
};

}  // namespace lol_at_home_server
