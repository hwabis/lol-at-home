#pragma once

#include "GameObject.h"
#include "IWorldRenderable.h"
#include "InputAccessor.h"

namespace lol_at_home_game {

class MyAwesomeGameObject : public lol_at_home_engine::GameObject {
 public:
  auto PushRender(
      std::vector<std::unique_ptr<lol_at_home_engine::IWorldRenderable>>&
          outRenderables) const -> void override {}
  auto Update(std::chrono::duration<double, std::milli> deltaTime,
              const lol_at_home_engine::InputAccessor& input) -> void override {
  }
};

}  // namespace lol_at_home_game
