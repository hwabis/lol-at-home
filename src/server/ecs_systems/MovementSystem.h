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
        registry
            .view<lol_at_home_shared::Position, lol_at_home_shared::Movable>();

    for (auto entity : view) {
      auto& pos = view.get<lol_at_home_shared::Position>(entity);
      auto& movable = view.get<lol_at_home_shared::Movable>(entity);

      if (movable.state != lol_at_home_shared::MovementState::Moving) {
        continue;
      }

      auto [newPos, reached] =
          moveTowards(pos, movable.targetPosition, movable.speed, timeElapsed);

      pos = newPos;

      if (reached) {
        movable.state = lol_at_home_shared::MovementState::Idle;
      }

      dirtyEntities.push_back(entity);
    }
  };

 private:
  static auto moveTowards(const lol_at_home_shared::Position& pos,
                          const lol_at_home_shared::Position& target,
                          double speed,
                          std::chrono::milliseconds timeElapsed)
      -> std::pair<lol_at_home_shared::Position, bool> {
    const double deltaX = target.x - pos.x;
    const double deltaY = target.y - pos.y;

    const double distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));
    if (distance == 0.0) {
      return {target, true};
    }

    const double seconds = std::chrono::duration<double>(timeElapsed).count();
    const double maxStep = speed * seconds;

    if (distance <= maxStep) {
      return {target, true};
    }

    lol_at_home_shared::Position newPos = pos;
    const double ratio = maxStep / distance;
    newPos.x += deltaX * ratio;
    newPos.y += deltaY * ratio;

    return {newPos, false};
  }
};

}  // namespace lol_at_home_server
