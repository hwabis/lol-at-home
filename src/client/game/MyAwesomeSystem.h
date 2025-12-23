#pragma once

#include <cmath>
#include "Components.h"
#include "IEcsSystem.h"

namespace lol_at_home_game {

class MyAwesomeSystem : public lol_at_home_engine::IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             lol_at_home_engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    // todo needs to use camera

    auto view = registry.view<Transform, RenderableCircle>();

    for (auto entity : view) {
      auto& transform = view.get<Transform>(entity);
      auto& circle = view.get<RenderableCircle>(entity);

      auto* renderer = info.sdlRenderer;

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      const float scaledRadius = circle.radius * transform.scale;

      const int intRadius = static_cast<int>(std::ceil(scaledRadius));

      const float baseY = transform.position.y;
      const float baseX = transform.position.x;

      for (int yDraw = -intRadius; yDraw <= intRadius; ++yDraw) {
        const auto yFloat = static_cast<float>(yDraw);
        const float deltaX =
            std::sqrt((scaledRadius * scaledRadius) - (yFloat * yFloat));

        SDL_RenderLine(renderer, baseX - deltaX, baseY + yFloat, baseX + deltaX,
                       baseY + yFloat);
      }
    }
  };
};

}  // namespace lol_at_home_game
