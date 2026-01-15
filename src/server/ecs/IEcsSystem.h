#pragma once

#include <chrono>
#include <entt/entt.hpp>

namespace lah::server {

class IEcsSystem {
 public:
  IEcsSystem() = default;
  virtual ~IEcsSystem() = default;

  IEcsSystem(const IEcsSystem&) = delete;
  auto operator=(const IEcsSystem&) -> IEcsSystem& = delete;
  IEcsSystem(IEcsSystem&&) = delete;
  auto operator=(IEcsSystem&&) -> IEcsSystem& = delete;

  // Push to dirtyInstant if need to push network update on this tick
  virtual void Cycle(entt::registry& registry,
                     std::chrono::milliseconds timeElapsed,
                     std::vector<entt::entity>& dirtyPeriodic,
                     std::vector<entt::entity>& dirtyInstant,
                     std::vector<entt::entity>& deletedEntities) = 0;

  [[nodiscard]] virtual auto GetPeriodicSyncRateHz() const -> int = 0;
};

}  // namespace lah::server
