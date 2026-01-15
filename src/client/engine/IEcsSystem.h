#pragma once

#include <chrono>
#include <entt/entt.hpp>
#include "SceneInfo.h"

namespace lah::engine {

class IEcsSystem {
 public:
  IEcsSystem() = default;
  virtual ~IEcsSystem() = default;

  IEcsSystem(const IEcsSystem&) = delete;
  auto operator=(const IEcsSystem&) -> IEcsSystem& = delete;
  IEcsSystem(IEcsSystem&&) = default;
  auto operator=(IEcsSystem&&) -> IEcsSystem& = default;

  virtual void Cycle(entt::registry& registry,
                     SceneInfo& info,
                     std::chrono::duration<double, std::milli> deltaTime) = 0;
};

}  // namespace lah::engine
