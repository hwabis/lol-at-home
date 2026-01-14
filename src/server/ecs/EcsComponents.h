#pragma once

#include <entt/entt.hpp>

namespace lol_at_home_server {

// keep in mind all structs here should be server-only.
// stuff client should also see goes in lol_at_home_shared

struct PendingDamage {
  entt::entity source;  // Who dealt the damage
  float amount;
};

struct PendingHeal {
  entt::entity source;  // Who healed
  float amount;
};

}  // namespace lol_at_home_server
