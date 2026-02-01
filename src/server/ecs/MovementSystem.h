#pragma once

#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs/EcsComponents.h"
#include "ecs/IEcsSystem.h"

namespace lah::server {

class MovementSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyPeriodic,
             std::vector<entt::entity>& dirtyInstant,
             std::vector<entt::entity>& /*deletedEntities*/) override {
    auto view = registry.view<lah::shared::Position, lah::shared::MovementStats,
                              lah::shared::MoveTarget>();
    for (auto entity : view) {
      auto& pos = view.get<lah::shared::Position>(entity);
      auto& movementStats = view.get<lah::shared::MovementStats>(entity);
      auto& moveTarget = view.get<lah::shared::MoveTarget>(entity);

      auto [newPos, reached] =
          moveTowards(pos, {.x = moveTarget.targetX, .y = moveTarget.targetY},
                      movementStats.speed, timeElapsed);

      pos = newPos;
      if (reached) {
        registry.remove<lah::shared::MoveTarget>(entity);

        auto* attackTarget = registry.try_get<AutoAttackTarget>(entity);
        if (attackTarget != nullptr) {
          auto* attackStats =
              registry.try_get<lah::shared::AutoAttackStats>(entity);
          if (attackStats != nullptr) {
            registry.emplace_or_replace<AutoAttackWindupTimer>(
                entity, AutoAttackWindupTimer{.remaining =
                                                  attackStats->windupDuration});
          }
        }

        // Send state change immediately when reaching destination
        dirtyInstant.push_back(entity);
      } else {
        dirtyPeriodic.push_back(entity);
      }
    }
  };

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 8;  // technically 7.5 for every 0.125s
    return rateHz;
  }

 private:
  static auto moveTowards(const lah::shared::Position& pos,
                          const lah::shared::Position& target,
                          float speed,
                          std::chrono::milliseconds timeElapsed)
      -> std::pair<lah::shared::Position, bool> {
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

    lah::shared::Position newPos = pos;
    const float ratio = maxStep / distance;
    newPos.x += deltaX * ratio;
    newPos.y += deltaY * ratio;

    return {newPos, false};
  }
};

}  // namespace lah::server
