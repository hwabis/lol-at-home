#pragma once

#include <chrono>
#include <entt/entt.hpp>
#include "IEcsSystem.h"

namespace lah::engine {

class Renderer;

class Scene {
 public:
  Scene() = default;
  virtual ~Scene() = default;

  Scene(const Scene&) = delete;
  auto operator=(const Scene&) -> Scene& = delete;
  Scene(Scene&&) = default;
  auto operator=(Scene&&) -> Scene& = default;

  auto Cycle(SceneInfo& info,
             std::chrono::duration<double, std::milli> deltaTime) -> void {
    for (auto& system : systems_) {
      system->Cycle(registry_, info, deltaTime);
    }
  }

  auto AddSystem(std::unique_ptr<IEcsSystem> system) -> void {
    systems_.push_back(std::move(system));
  }

  auto GetRegistry() -> entt::registry& { return registry_; }

 private:
  entt::registry registry_;
  std::vector<std::unique_ptr<IEcsSystem>> systems_;
};

}  // namespace lah::engine
