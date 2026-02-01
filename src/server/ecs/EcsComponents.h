#pragma once

#include <entt/entt.hpp>

namespace lah::server {

// keep in mind all structs here should be server-only.
// stuff client should also see goes in lah::shared

struct PendingDamage {
  entt::entity source;  // Who dealt the damage
  float amount;
};

struct PendingHeal {
  entt::entity source;  // Who healed
  float amount;
};

// transient component - only attached during aa move or aa windup
struct AutoAttackTarget {
  entt::entity target;
};

// transient component - only attached during windup
struct AutoAttackWindupTimer {
  float remaining;
};

}  // namespace lah::server
