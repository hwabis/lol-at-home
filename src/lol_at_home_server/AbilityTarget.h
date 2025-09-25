#pragma once

#include <entt/entt.hpp>
#include <variant>
#include "EcsComponents.h"

namespace lol_at_home_server {

struct NoTarget {};

struct EntityTarget {
  entt::entity Target;
};

struct OnePointSkillshot {
  Position Target;  // Relative to caster
};

// Rumble R, Viktor E
struct TwoPointSkillshot {
  Position Target1;  // Relative to caster
  Position Target2;  // Relative to caster
};

using AbilityTargetVariant =
    std::variant<NoTarget, EntityTarget, OnePointSkillshot, TwoPointSkillshot>;

}  // namespace lol_at_home_server
