#pragma once

#include <entt/entt.hpp>
#include "AbilityTarget.h"
#include "EcsComponents.h"

namespace lol_at_home_server {

class AbilityImpl {
 public:
  virtual ~AbilityImpl() = default;
  AbilityImpl(const AbilityImpl&) = default;
  auto operator=(const AbilityImpl&) -> AbilityImpl& = default;
  AbilityImpl(AbilityImpl&&) = default;
  auto operator=(AbilityImpl&&) -> AbilityImpl& = default;

  virtual void Execute(entt::registry& registry,
                       const Abilities::Ability& abilityData,
                       const AbilityTargetVariant& target) = 0;
};

}  // namespace lol_at_home_server
