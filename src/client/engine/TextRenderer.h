#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include "Vector2.h"

namespace lah::engine {

class TextRenderer {
 public:
  static void DrawText(SDL_Renderer* renderer,
                       TTF_Font* font,
                       const std::string& text,
                       Vector2 pos,
                       SDL_Color color = {255, 255, 255, 255}) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), 0, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture != nullptr) {
      SDL_FRect destRect{pos.x, pos.y, static_cast<float>(surface->w),
                         static_cast<float>(surface->h)};
      SDL_RenderTexture(renderer, texture, nullptr, &destRect);
      SDL_DestroyTexture(texture);
    }

    SDL_DestroySurface(surface);
  }
};

}  // namespace lah::engine
