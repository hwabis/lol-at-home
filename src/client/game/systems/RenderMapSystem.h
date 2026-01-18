#pragma once

#include <SDL3/SDL_render.h>
#include "IEcsSystem.h"
#include "domain/ArenaConfig.h"

namespace lah::game {

class RenderMapSystem : public lah::engine::IEcsSystem {
 public:
  void Cycle(entt::registry& /*registry*/,
             lah::engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    drawArena(info);
  }

 private:
  static void drawArena(lah::engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;

    lah::engine::Vector2 topLeft{.x = lah::shared::ARENA_MIN_X,
                                 .y = lah::shared::ARENA_MIN_Y};
    lah::engine::Vector2 bottomRight{.x = lah::shared::ARENA_MAX_X,
                                     .y = lah::shared::ARENA_MAX_Y};

    auto screenTopLeft = info.camera.WorldToScreen(topLeft);
    auto screenBottomRight = info.camera.WorldToScreen(bottomRight);

    float width = screenBottomRight.x - screenTopLeft.x;
    float height = screenBottomRight.y - screenTopLeft.y;

    SDL_FRect arenaRect{screenTopLeft.x, screenTopLeft.y, width, height};

    SDL_SetRenderDrawColor(renderer, 0, 128, 0, 255);  // Green
    SDL_RenderFillRect(renderer, &arenaRect);
  }
};

}  // namespace lah::game
