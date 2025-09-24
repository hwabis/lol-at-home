#pragma once

#include <entt/entt.hpp>
#include "GameAction.h"
#include "GameStateDelta.h"

namespace lol_at_home_server {

class GameState {
 public:
  auto ProcessActionsAndUpdate(const std::vector<GameActionVariant>& actions,
                               double deltaTimeMs) -> GameStateDelta;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  entt::registry Registry;

 private:
  void updateMovementSystem(double deltaTimeMs);
  void updateHealthSystem(double deltaTimeMs);
};

}  // namespace lol_at_home_server
