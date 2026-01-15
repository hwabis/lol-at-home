#pragma once

#include <algorithm>
#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs/EcsComponents.h"
#include "ecs/IEcsSystem.h"

namespace lah::server {

class HealthSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyPeriodic,
             std::vector<entt::entity>& dirtyInstant,
             std::vector<entt::entity>& deletedEntities) override {
    auto view = registry.view<lah::shared::Health>();

    for (auto entity : view) {
      auto& health = view.get<lah::shared::Health>(entity);

      if (auto* healthEvent = registry.try_get<PendingDamage>(entity)) {
        health.current -= healthEvent->amount;
        dirtyInstant.push_back(entity);
        registry.remove<PendingDamage>(entity);
      }

      if (auto* healthEvent = registry.try_get<PendingHeal>(entity)) {
        health.current += healthEvent->amount;
        health.current = std::min(health.current, health.max);
        dirtyInstant.push_back(entity);
        registry.remove<PendingHeal>(entity);
      }

      if (health.current <= 0) {
        deletedEntities.push_back(entity);
        registry.destroy(entity);
        continue;
      }

      if (health.current < health.max) {
        const float seconds = std::chrono::duration<float>(timeElapsed).count();
        health.current = std::min(
            health.max, health.current + (seconds * health.regenPerSec));
        dirtyPeriodic.push_back(entity);
      }
    }
  }

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 2;
    return rateHz;
  }
};

}  // namespace lah::server
