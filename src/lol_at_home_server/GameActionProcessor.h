#pragma once

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
#include "AbilityImpl.h"
#include "GameAction.h"

namespace lol_at_home_server {

class GameActionProcessor {
 public:
  explicit GameActionProcessor(entt::registry& registry)
      : registry_(registry) {}

  void operator()(const MoveAction& action) {
    if (!registry_.valid(action.Source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    if (!registry_.all_of<Movable>(action.Source)) {
      spdlog::warn("Entity {} has no Movable component",
                   static_cast<uint32_t>(action.Source));
      return;
    }

    registry_.emplace_or_replace<Moving>(action.Source, action.TargetPosition);
  }

  void operator()(const AbilityAction& action) {
    if (!registry_.valid(action.Source)) {
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    auto* abilities = registry_.try_get<Abilities>(action.Source);
    if (abilities == nullptr) {
      spdlog::warn("Entity {} has no Abilities component",
                   static_cast<uint32_t>(action.Source));
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

    auto abilityImpl = getAbilityImpl(ability.Id);
    abilityImpl->Execute(registry_, ability, action.Target);
  }

  void operator()(const AutoAttackAction& action) {
    // todo
  }

  void operator()(const StopGameAction& action) {
    // todo
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  entt::registry& registry_;

  // todo maybe make this into ability registry class
  static auto getAbilityImpl(AbilityId abilityId)
      -> std::unique_ptr<AbilityImpl> {
    // The reason we do to a switch instead of a map is to cover all AbilityId
    // at compile time (compiler warning otherwise)
    switch (abilityId) {
      // todo
      // in fact the compiler is emitting a warning right nao
    }

    throw std::runtime_error("Unhandled AbilityId: " +
                             std::to_string(static_cast<int>(abilityId)));
  }
};

}  // namespace lol_at_home_server
