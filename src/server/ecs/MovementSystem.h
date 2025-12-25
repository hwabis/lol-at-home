#pragma once

#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs/IEcsSystem.h"

namespace lol_at_home_server {

class MovementSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyPeriodic,
             std::vector<entt::entity>& /*dirtyInstant*/,
             std::vector<entt::entity>& /*deletedEntities*/) override {
    auto view =
        registry
            .view<lol_at_home_shared::Position, lol_at_home_shared::Movable>();
    for (auto entity : view) {
      auto& pos = view.get<lol_at_home_shared::Position>(entity);
      auto& movable = view.get<lol_at_home_shared::Movable>(entity);

      auto [newPos, reached] =
          moveTowards(pos, movable.targetPosition, movable.speed, timeElapsed);

      pos = newPos;
      movable.state = reached ? lol_at_home_shared::MovementState::Idle
                              : lol_at_home_shared::MovementState::Moving;

      // Technically movement system never sends updates instantly (we currently
      // have no way of knowing when the target position has changed, aka the
      // player right-clicked), but the sync rate is so high anyway. Not sure
      // what LoL actually does
      dirtyPeriodic.push_back(entity);
    }
  };

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 8;  // technically 7.5 for every 0.125s
    return rateHz;
  }

 private:
  static auto moveTowards(const lol_at_home_shared::Position& pos,
                          const lol_at_home_shared::Position& target,
                          float speed,
                          std::chrono::milliseconds timeElapsed)
      -> std::pair<lol_at_home_shared::Position, bool> {
    const float deltaX = target.x - pos.x;
    const float deltaY = target.y - pos.y;

    const float distance = std::sqrt((deltaX * deltaX) + (deltaY * deltaY));
    if (distance == 0.0) {
      return {target, true};
    }

    const float seconds = std::chrono::duration<float>(timeElapsed).count();
    const float maxStep = speed * seconds;

    if (distance <= maxStep) {
      return {target, true};
    }

    lol_at_home_shared::Position newPos = pos;
    const float ratio = maxStep / distance;
    newPos.x += deltaX * ratio;
    newPos.y += deltaY * ratio;

    return {newPos, false};
  }
};

}  // namespace lol_at_home_server
