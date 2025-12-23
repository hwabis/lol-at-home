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

    auto* renderer = info.sdlRenderer;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    auto view = registry.view<Transform, RenderableCircle>();
    for (auto entity : view) {
      auto& transform = view.get<Transform>(entity);
      auto& circle = view.get<RenderableCircle>(entity);

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      float radius = circle.radius * transform.scale;
      float centerX = transform.position.x;
      float centerY = transform.position.y;

      int intRadius = static_cast<int>(std::ceil(radius));
      for (int yDraw = -intRadius; yDraw <= intRadius; yDraw++) {
        float width = std::sqrt((radius * radius) - (yDraw * yDraw));
        SDL_RenderLine(renderer, centerX - width,
                       centerY + static_cast<float>(yDraw), centerX + width,
                       centerY + static_cast<float>(yDraw));
      }
    }

    // Present to screen
    SDL_RenderPresent(renderer);
  }
};

}  // namespace lol_at_home_game
