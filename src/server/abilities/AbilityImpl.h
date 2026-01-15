#pragma once

#include <entt/entt.hpp>
#include "abilities/AbilityTarget.h"
#include "domain/EcsComponents.h"

namespace lah::server {

class AbilityImpl {
 public:
  virtual ~AbilityImpl() = default;
  AbilityImpl(const AbilityImpl&) = default;
  auto operator=(const AbilityImpl&) -> AbilityImpl& = default;
  AbilityImpl(AbilityImpl&&) = default;
  auto operator=(AbilityImpl&&) -> AbilityImpl& = default;

  virtual void Execute(entt::registry& registry,
                       const lah::shared::Abilities::Ability& ability,
                       const lah::shared::AbilityTargetVariant& target) = 0;
};

}  // namespace lah::server
