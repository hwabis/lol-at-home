#pragma once

#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs/EcsComponents.h"
#include "ecs/IEcsSystem.h"

namespace lah::server {

class AutoAttackSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& /*dirtyPeriodic*/,
             std::vector<entt::entity>& dirtyInstant,
             std::vector<entt::entity>& /*deletedEntities*/) override {
    const float seconds = std::chrono::duration<float>(timeElapsed).count();

    auto view =
        registry.view<lah::shared::CharacterState, AutoAttackWindupTimer,
                      AutoAttackTarget, lah::shared::AutoAttackStats>();

    for (auto entity : view) {
      auto& characterState = view.get<lah::shared::CharacterState>(entity);

      if (characterState.state !=
          lah::shared::CharacterState::State::AutoAttackWindup) {
        registry.remove<AutoAttackWindupTimer>(entity);
        registry.remove<AutoAttackTarget>(entity);
        continue;
      }

      auto& windupTimer = view.get<AutoAttackWindupTimer>(entity);
      auto& attackTarget = view.get<AutoAttackTarget>(entity);
      auto& attackStats = view.get<lah::shared::AutoAttackStats>(entity);

      windupTimer.remaining -= seconds;

      if (windupTimer.remaining <= 0.0F) {
        if (registry.valid(attackTarget.target)) {
          registry.emplace_or_replace<PendingDamage>(
              attackTarget.target,
              PendingDamage{.source = entity, .amount = attackStats.damage});
          dirtyInstant.push_back(attackTarget.target);
        }

        characterState.state = lah::shared::CharacterState::State::Idle;
        registry.remove<AutoAttackWindupTimer>(entity);
        registry.remove<AutoAttackTarget>(entity);
        dirtyInstant.push_back(entity);
      }
    }
  }

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 30;  // High rate for responsive auto-attacks
    return rateHz;
  }
};

}  // namespace lah::server
