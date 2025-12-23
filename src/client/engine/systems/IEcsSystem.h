#pragma once

#include <chrono>
#include <entt/entt.hpp>

namespace lol_at_home_engine {

class IEcsSystem {
 public:
  virtual ~IEcsSystem() = default;

  IEcsSystem(const IEcsSystem&) = delete;
  auto operator=(const IEcsSystem&) -> IEcsSystem& = delete;
  IEcsSystem(IEcsSystem&&) = default;
  auto operator=(IEcsSystem&&) -> IEcsSystem& = default;

  virtual void Cycle(entt::registry& registry,
                     std::chrono::duration<double, std::milli> deltaTime) = 0;
};

}  // namespace lol_at_home_engine
