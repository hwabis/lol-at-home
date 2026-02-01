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

    auto view = registry.view<AutoAttackWindupTimer, AutoAttackTarget,
                              lah::shared::AutoAttackStats>();

    for (auto entity : view) {
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

        registry.remove<AutoAttackWindupTimer>(entity);
        registry.remove<AutoAttackTarget>(entity);
        dirtyInstant.push_back(entity);
      }
    }
  }

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 30;
    return rateHz;
  }
};

}  // namespace lah::server
