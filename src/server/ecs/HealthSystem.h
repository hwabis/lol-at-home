#pragma once

#include <algorithm>
#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs/EcsComponents.h"
#include "ecs/IEcsSystem.h"

namespace lol_at_home_server {

class HealthSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyPeriodic,
             std::vector<entt::entity>& dirtyInstant,
             std::vector<entt::entity>& deletedEntities) override {
    auto view = registry.view<lol_at_home_shared::Health>();

    for (auto entity : view) {
      auto& health = view.get<lol_at_home_shared::Health>(entity);

      if (auto* healthEvent = registry.try_get<PendingDamage>(entity)) {
        health.currentHealth -= healthEvent->amount;
        dirtyInstant.push_back(entity);
        registry.remove<PendingDamage>(entity);
      }

      if (auto* healthEvent = registry.try_get<PendingHeal>(entity)) {
        health.currentHealth += healthEvent->amount;
        health.currentHealth = std::min(health.currentHealth, health.maxHealth);
        dirtyInstant.push_back(entity);
        registry.remove<PendingHeal>(entity);
      }

      if (health.currentHealth <= 0) {
        deletedEntities.push_back(entity);
        registry.destroy(entity);
        continue;
      }

      if (health.currentHealth < health.maxHealth) {
        const float seconds = std::chrono::duration<float>(timeElapsed).count();
        health.currentHealth = std::min(
            health.maxHealth,
            health.currentHealth + (seconds * health.healthRegenPerSec));
        dirtyPeriodic.push_back(entity);
      }
    }
  }

  [[nodiscard]] auto GetPeriodicSyncRateHz() const -> int override {
    constexpr int rateHz = 2;
    return rateHz;
  }
};

}  // namespace lol_at_home_server
