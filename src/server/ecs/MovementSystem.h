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
             std::vector<entt::entity>& /*dirtyInstant*/,
             std::vector<entt::entity>& /*deletedEntities*/) override {
    auto view =
        registry.view<lah::shared::Position, lah::shared::MovementStats,
                      lah::shared::CharacterState, lah::shared::MoveTarget>();
    for (auto entity : view) {
      auto& pos = view.get<lah::shared::Position>(entity);
      auto& characterState = view.get<lah::shared::CharacterState>(entity);
      auto& movementStats = view.get<lah::shared::MovementStats>(entity);
      auto& moveTarget = view.get<lah::shared::MoveTarget>(entity);

      auto [newPos, reached] =
          moveTowards(pos, {.x = moveTarget.targetX, .y = moveTarget.targetY},
                      movementStats.speed, timeElapsed);

      pos = newPos;
      if (reached) {
        registry.remove<lah::shared::MoveTarget>(entity);

        if (characterState.state ==
            lah::shared::CharacterState::State::Moving) {
          characterState.state = lah::shared::CharacterState::State::Idle;
        } else if (characterState.state ==
                   lah::shared::CharacterState::State::AutoAttackMoving) {
          characterState.state =
              lah::shared::CharacterState::State::AutoAttackWindup;
          auto* attackStats =
              registry.try_get<lah::shared::AutoAttackStats>(entity);
          if (attackStats != nullptr) {
            registry.emplace_or_replace<AutoAttackWindupTimer>(
                entity, AutoAttackWindupTimer{.remaining =
                                                  attackStats->windupDuration});
          }
        }
      }

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
