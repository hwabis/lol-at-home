#pragma once

#include <spdlog/spdlog.h>
#include <entt/entt.hpp>
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
      spdlog::warn("Attempted to process action on invalid entity");
      return;
    }

    registry_.emplace_or_replace<Moving>(action.Source, action.TargetPosition);
  }

  void operator()(const AbilityGameAction& action) {
    // todo
  }

  void operator()(const AutoAttackGameAction& action) {
    // todo
  }

  void operator()(const StopGameAction& action) {
    // todo
  }

 private:
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
  entt::registry& registry_;
};

}  // namespace lol_at_home_server
