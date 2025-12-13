#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include "IWorldRenderable.h"
#include "InputAccessor.h"
#include "primitives/Transform.h"

namespace lol_at_home_engine {

class GameObject {
 public:
  virtual ~GameObject() = default;
  GameObject(const GameObject&) = delete;
  auto operator=(const GameObject&) -> GameObject& = delete;
  GameObject(GameObject&& other) = delete;
  auto operator=(GameObject&&) -> GameObject& = delete;

  virtual auto PushRender(
      std::vector<std::unique_ptr<IWorldRenderable>>& outRenderables) const
      -> void = 0;
  virtual auto Update(std::chrono::duration<double, std::milli> deltaTime,
                      const InputAccessor& input) -> void = 0;

  [[nodiscard]] auto GetTransform() const -> Transform { return transform_; }

 private:
  Transform transform_;
};

}  // namespace lol_at_home_engine
