#pragma once

#include <chrono>
#include <entt/entt.hpp>
#include "systems/IEcsSystem.h"

namespace lol_at_home_engine {

class Renderer;

class Scene {
 public:
  Scene();
  virtual ~Scene() = default;

  Scene(const Scene&) = delete;
  auto operator=(const Scene&) -> Scene& = delete;
  Scene(Scene&&) = delete;
  auto operator=(Scene&&) -> Scene& = delete;

  auto Cycle(std::chrono::duration<double, std::milli> deltaTime) -> void {
    for (auto& system : systems_) {
      system->Cycle(registry_, deltaTime);
    }
  }

  auto AddSystem(std::unique_ptr<IEcsSystem> system) -> void {
    systems_.push_back(std::move(system));
  }

 private:
  entt::registry registry_;
  std::vector<std::unique_ptr<IEcsSystem>> systems_;
};

}  // namespace lol_at_home_engine
