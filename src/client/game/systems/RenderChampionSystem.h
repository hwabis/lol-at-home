#pragma once

#include <SDL3/SDL_render.h>
#include <spdlog/spdlog.h>
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
    auto view = registry.view<lah::shared::Position, lah::shared::Radius>();
    for (auto entity : view) {
      auto& position = view.get<lah::shared::Position>(entity);
      auto& radius = view.get<lah::shared::Radius>(entity);

      lah::engine::Vector2 worldPos{.x = position.x, .y = position.y};
      auto screenPos = info.camera.WorldToScreen(worldPos);

      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

      float centerX = screenPos.x;
      float centerY = screenPos.y;

      int intRadius = static_cast<int>(radius.radius);
      for (int yDraw = -intRadius; yDraw <= intRadius; yDraw++) {
        auto width = static_cast<float>(
            std::sqrt((intRadius * intRadius) - (yDraw * yDraw)));
        SDL_RenderLine(renderer, centerX - width,
                       centerY + static_cast<float>(yDraw), centerX + width,
                       centerY + static_cast<float>(yDraw));
      }
    }
  }

  static void drawHealthBars(entt::registry& registry,
                             lah::engine::SceneInfo& info) {
    auto* renderer = info.sdlRenderer;
    auto view = registry.view<lah::shared::Position, lah::shared::Radius,
                              lah::shared::Health, lah::shared::Team>();
    for (auto entity : view) {
      auto& position = view.get<lah::shared::Position>(entity);
      auto& radius = view.get<lah::shared::Radius>(entity);
      auto& health = view.get<lah::shared::Health>(entity);
      auto& team = view.get<lah::shared::Team>(entity);

      lah::engine::Vector2 worldPos{.x = position.x, .y = position.y};
      auto screenPos = info.camera.WorldToScreen(worldPos);

      float healthRatio = std::clamp(health.current / health.max, 0.0F, 1.0F);
      constexpr float barWidth = 100.0F;
      constexpr float barHeight = 10.0F;
      float barX = screenPos.x - barWidth * 0.5F;
      float barY = screenPos.y - radius.radius - barHeight - 10.0F;

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
        registry.view<lah::shared::Position, lah::shared::Radius,
                      lah::shared::CharacterState, lah::shared::ChampionType>();

    constexpr float barWidth = 100.0F;
    constexpr float barHeight = 10.0F;
    constexpr float textOffsetAboveBar = 5.0F;

    for (auto entity : view) {
      auto& position = view.get<lah::shared::Position>(entity);
      auto& radius = view.get<lah::shared::Radius>(entity);
      auto& state = view.get<lah::shared::CharacterState>(entity);
      auto& championType = view.get<lah::shared::ChampionType>(entity);

      lah::engine::Vector2 worldPos{.x = position.x, .y = position.y};
      auto screenPos = info.camera.WorldToScreen(worldPos);

      std::string displayText;
      if (state.state == lah::shared::CharacterState::State::Idle) {
        displayText = getChampionName(championType.id);
      } else {
        displayText = getStateText(state.state);
      }

      float textX = screenPos.x - barWidth * 0.5F;
      float textY =
          screenPos.y - radius.radius - barHeight - 10.0F - textOffsetAboveBar;
      lah::engine::TextRenderer::DrawText(renderer, font, displayText,
                                          {.x = textX, .y = textY});
    }
  }

  static auto getChampionName(lah::shared::ChampionId id) -> std::string {
    switch (id) {
      case lah::shared::ChampionId::Garen:
        return "Garen";
      case lah::shared::ChampionId::Teemo:
        return "Teemo";
    }

    // unreachable (compiler catches all switch cases)
    return "Unknown";
  }

  static auto getStateText(lah::shared::CharacterState::State state)
      -> std::string {
    switch (state) {
      case lah::shared::CharacterState::State::Idle:
        return "IDLE";
      case lah::shared::CharacterState::State::Moving:
        return "MOVING";
      case lah::shared::CharacterState::State::AutoAttackMoving:
        return "AUTO ATTACK MOVING";
      case lah::shared::CharacterState::State::AutoAttackWindup:
        return "AUTO ATTACK WINDUP";
    }

    // unreachable (compiler catches all switch cases)
    return "UNKNOWN";
  }
};

}  // namespace lah::game
