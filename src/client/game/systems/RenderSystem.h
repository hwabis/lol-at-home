#pragma once

#include <SDL3/SDL_render.h>
#include "Components.h"
#include "IEcsSystem.h"

namespace lol_at_home_game {

class RenderSystem : public lol_at_home_engine::IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             lol_at_home_engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    auto* renderer = info.sdlRenderer;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    drawChampions(registry, info);
    drawHealthBars(registry, info);

    SDL_RenderPresent(renderer);
  }

 private:
  static void drawChampions(entt::registry& registry,
                            lol_at_home_engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto view = registry.view<Transform, RenderableCircle>();
    for (auto entity : view) {
      auto& transform = view.get<Transform>(entity);
      auto& circle = view.get<RenderableCircle>(entity);

      auto screenPos = info.camera.WorldToScreen(transform.worldPosition);

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      float radius = circle.radius * transform.scale;
      float centerX = screenPos.x;
      float centerY = screenPos.y;

      int intRadius = static_cast<int>(std::ceil(radius));
      for (int yDraw = -intRadius; yDraw <= intRadius; yDraw++) {
        float width = std::sqrt((radius * radius) - (yDraw * yDraw));
        SDL_RenderLine(renderer, centerX - width,
                       centerY + static_cast<float>(yDraw), centerX + width,
                       centerY + static_cast<float>(yDraw));
      }
    }
  }

  static void drawHealthBars(entt::registry& registry,
                             lol_at_home_engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto view = registry.view<Transform, RenderableCircle, Health, Team>();
    for (auto entity : view) {
      auto& transform = view.get<Transform>(entity);
      auto& circle = view.get<RenderableCircle>(entity);
      auto& health = view.get<Health>(entity);
      auto& team = view.get<Team>(entity);

      auto screenPos = info.camera.WorldToScreen(transform.worldPosition);

      float healthRatio = std::clamp(health.current / health.max, 0.0F, 1.0F);
      constexpr float barWidth = 100.0F;
      constexpr float barHeight = 10.0F;
      float barX = screenPos.x - barWidth * 0.5F;
      float barY = screenPos.y - circle.radius - barHeight - 10.0F;

      SDL_FRect background{barX, barY, barWidth, barHeight};
      SDL_FRect foreground{background.x, background.y, barWidth * healthRatio,
                           background.h};

      SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
      SDL_RenderFillRect(renderer, &background);

      switch (team.color) {
        case Team::Color::Blue:
          SDL_SetRenderDrawColor(renderer, 0, 122, 255, 255);
          break;
        case Team::Color::Red:
          SDL_SetRenderDrawColor(renderer, 255, 59, 48, 255);
          break;
      }
      SDL_RenderFillRect(renderer, &foreground);
    }
  }
};

}  // namespace lol_at_home_game
