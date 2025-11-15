#pragma once

#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs_systems/IEcsSystem.h"

namespace lol_at_home_server {

class MovementSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyEntities,
             std::vector<entt::entity>& /*deletedEntities*/) override {
    auto view =
        registry.view<lol_at_home_shared::Position, lol_at_home_shared::Movable,
                      lol_at_home_shared::Moving>();

    for (auto entity : view) {
      auto& pos = view.get<lol_at_home_shared::Position>(entity);
      auto& movable = view.get<lol_at_home_shared::Movable>(entity);
      auto& moving = view.get<lol_at_home_shared::Moving>(entity);

      bool reached =
          moveTowards(pos, moving.targetPosition, movable.speed, timeElapsed);

      if (reached) {
        registry.erase<lol_at_home_shared::Moving>(entity);
      }

      dirtyEntities.push_back(entity);
    }
  };

 private:
  static auto moveTowards(lol_at_home_shared::Position& pos,
                          const lol_at_home_shared::Position& target,
                          double speed,
                          std::chrono::milliseconds timeElapsed) -> bool {
    const double deltaX = target.x - pos.x;
    const double deltaY = target.y - pos.y;

    const double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));
    if (distance == 0.0) {
      return true;
    }

    const double seconds = std::chrono::duration<double>(timeElapsed).count();
    const double maxStep = speed * seconds;

    if (distance <= maxStep) {
      pos = target;
      return true;
    }

    const double ratio = maxStep / distance;
    pos.x += deltaX * ratio;
    pos.y += deltaY * ratio;

    return false;
  }
};

}  // namespace lol_at_home_server
