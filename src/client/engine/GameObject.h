#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include "IWorldRenderable.h"
#include "primitives/Transform.h"

namespace lol_at_home_engine {

class GameObject {
 public:
  virtual ~GameObject() = default;
  GameObject(const GameObject&) = delete;
  auto operator=(const GameObject&) -> GameObject& = delete;
  GameObject(GameObject&& other) = delete;
  auto operator=(GameObject&&) -> GameObject& = delete;

  [[nodiscard]] virtual auto Render() const
      -> std::vector<std::unique_ptr<IWorldRenderable>> = 0;
  virtual auto Update(std::chrono::duration<double, std::milli> deltaTime)
      -> void = 0;

  [[nodiscard]] auto GetTransform() const -> Transform { return transform_; }

 private:
  Transform transform_;
};

}  // namespace lol_at_home_engine
