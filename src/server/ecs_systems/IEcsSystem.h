#pragma once

#include <chrono>
#include <entt/entt.hpp>

namespace lol_at_home_server {

class IEcsSystem {
 public:
  IEcsSystem() = default;
  virtual ~IEcsSystem() = default;

  IEcsSystem(const IEcsSystem&) = delete;
  auto operator=(const IEcsSystem&) -> IEcsSystem& = delete;
  IEcsSystem(IEcsSystem&&) = delete;
  auto operator=(IEcsSystem&&) -> IEcsSystem& = delete;

  virtual void Cycle(entt::registry& registry,
                     std::chrono::milliseconds timeElapsed,
                     std::vector<entt::entity>& dirtyEntities,
                     std::vector<entt::entity>& deletedEntities) = 0;
};

}  // namespace lol_at_home_server
