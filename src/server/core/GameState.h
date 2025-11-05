#pragma once

#include <entt/entt.hpp>
#include <mutex>
#include "GameAction.h"
#include "core/GameStateDelta.h"

namespace lol_at_home_server {

class GameState {
 public:
  auto ProcessActionsAndUpdate(
      const std::vector<lol_at_home_shared::GameActionVariant>& actions,
      double deltaTimeMs) -> GameStateDelta;
  auto CreatePlayerEntity() -> entt::entity;
  auto SerializeFullState() -> std::vector<std::byte>;

  entt::registry Registry;

 private:
  void updateMovementSystem(double deltaTimeMs,
                            std::vector<entt::entity>& dirtyEntities);
  void updateHealthSystem(double deltaTimeMs,
                          std::vector<entt::entity>& dirtyEntities);

  std::mutex registryMutex_;
};

}  // namespace lol_at_home_server
