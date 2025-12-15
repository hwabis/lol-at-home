#pragma once

#include "GameObject.h"
#include "InputAccessor.h"
#include "renderables/CircleRenderable.h"
#include "renderables/IWorldRenderable.h"

namespace lol_at_home_game {

class MyAwesomeGameObject : public lol_at_home_engine::GameObject {
 public:
  auto PushRender(
      std::vector<std::unique_ptr<lol_at_home_engine::IWorldRenderable>>&
          outRenderables) const -> void override {
    outRenderables.push_back(
        std::make_unique<lol_at_home_engine::CircleRenderable>(
            GetTransform(), 100.0f,
            lol_at_home_engine::Color{.r = 255, .g = 0, .b = 0, .a = 255}));
  }

  auto Update(std::chrono::duration<double, std::milli> deltaTime,
              const lol_at_home_engine::InputAccessor& input) -> void override {
  }
};

}  // namespace lol_at_home_game
