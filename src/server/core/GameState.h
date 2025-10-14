#pragma once

#include <entt/entt.hpp>
#include "actions/GameAction.h"
#include "core/GameStateDelta.h"

namespace lol_at_home_server {

class GameState {
 public:
  auto ProcessActionsAndUpdate(const std::vector<GameActionVariant>& actions,
                               double deltaTimeMs) -> GameStateDelta;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  entt::registry Registry;

 private:
  void updateMovementSystem(double deltaTimeMs,
                            std::vector<entt::entity>& dirtyEntities);
  void updateHealthSystem(double deltaTimeMs,
                          std::vector<entt::entity>& dirtyEntities);
};

}  // namespace lol_at_home_server
