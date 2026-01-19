#pragma once

#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>
#include "Components.h"
#include "IEcsSystem.h"
#include "TextRenderer.h"
#include "domain/EcsComponents.h"

namespace lah::game {

class RenderChampionSystem : public lah::engine::IEcsSystem {
 public:
  void Cycle(entt::registry& registry,
             lah::engine::SceneInfo& info,
             std::chrono::duration<double, std::milli> /*deltaTime*/) override {
    drawChampions(registry, info);
    drawHealthBars(registry, info);
    drawCharacterState(registry, info);
  }

 private:
  static void drawChampions(entt::registry& registry,
                            lah::engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto view = registry.view<lah::shared::Position>();
    for (auto entity : view) {
      auto& position = view.get<lah::shared::Position>(entity);

      lah::engine::Vector2 worldPos{.x = position.x, .y = position.y};
      auto screenPos = info.camera.WorldToScreen(worldPos);

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      float radius = position.championRadius;
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
                             lah::engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto view = registry.view<lah::shared::Position, lah::shared::Health,
                              lah::shared::Team>();
    for (auto entity : view) {
      auto& position = view.get<lah::shared::Position>(entity);
      auto& health = view.get<lah::shared::Health>(entity);
      auto& team = view.get<lah::shared::Team>(entity);

      lah::engine::Vector2 worldPos{.x = position.x, .y = position.y};
      auto screenPos = info.camera.WorldToScreen(worldPos);

      float healthRatio = std::clamp(health.current / health.max, 0.0F, 1.0F);
      constexpr float barWidth = 100.0F;
      constexpr float barHeight = 10.0F;
      float barX = screenPos.x - barWidth * 0.5F;
      float barY = screenPos.y - position.championRadius - barHeight - 10.0F;

      SDL_FRect background{barX, barY, barWidth, barHeight};
      SDL_FRect foreground{background.x, background.y, barWidth * healthRatio,
                           background.h};

      SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
      SDL_RenderFillRect(renderer, &background);

      switch (team.color) {
        case lah::shared::Team::Color::Blue:
          SDL_SetRenderDrawColor(renderer, 8, 24, 168, 255);
          break;
        case lah::shared::Team::Color::Red:
          SDL_SetRenderDrawColor(renderer, 255, 59, 48, 255);
          break;
      }
      SDL_RenderFillRect(renderer, &foreground);
    }
  }

  static void drawCharacterState(entt::registry& registry,
                                 lah::engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto* font = info.font;

    auto view =
        registry.view<lah::game::LocalPlayer, lah::shared::CharacterState>();

    bool foundPlayer = false;
    for (auto entity : view) {
      foundPlayer = true;
      auto& state = view.get<lah::shared::CharacterState>(entity);

      std::string stateText;
      switch (state.state) {
        case lah::shared::CharacterState::State::Idle:
          stateText = "Idle";
          break;
        case lah::shared::CharacterState::State::Moving:
          stateText = "Moving";
          break;
        case lah::shared::CharacterState::State::AutoAttackWindup:
          stateText = "Auto Attack Windup";
          break;
      }

      std::string displayText = std::string("State: ") + stateText;
      lah::engine::TextRenderer::DrawText(renderer, font, displayText,
                                          {.x = 10, .y = 10});

      break;
    }

    if (!foundPlayer) {
      static bool loggedOnce = false;
      if (!loggedOnce) {
        spdlog::warn(
            "No LocalPlayer entity with CharacterState found for text "
            "rendering");
        loggedOnce = true;
      }
    }
  }
};

}  // namespace lah::game
