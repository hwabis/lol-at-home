#pragma once

#include <entt/entt.hpp>

namespace lol_at_home_server {

struct PendingDamage {
  entt::entity source;  // Who dealt the damage
  double amount;
};

struct PendingHeal {
  entt::entity source;  // Who healed
  double amount;
};

}  // namespace lol_at_home_server
