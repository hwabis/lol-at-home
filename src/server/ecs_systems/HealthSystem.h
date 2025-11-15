#pragma once

#include <chrono>
#include "domain/EcsComponents.h"
#include "ecs_systems/IEcsSystem.h"

namespace lol_at_home_server {

class HealthSystem : public IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             std::chrono::milliseconds timeElapsed,
             std::vector<entt::entity>& dirtyEntities,
             std::vector<entt::entity>& deletedEntities) override {
    auto view = registry.view<lol_at_home_shared::Health>();

    for (auto entity : view) {
      auto& health = view.get<lol_at_home_shared::Health>(entity);

      if (health.currentHealth < health.maxHealth) {
        const double seconds =
            std::chrono::duration<double>(timeElapsed).count();
        health.currentHealth = std::min(
            health.maxHealth,
            health.currentHealth + (seconds * health.healthRegenPerSec));
        dirtyEntities.push_back(entity);
      } else if (health.currentHealth <= 0) {
        deletedEntities.push_back(entity);
        registry.destroy(entity);
      }
    }
  }
};

}  // namespace lol_at_home_server
