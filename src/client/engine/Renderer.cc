#include "Renderer.h"
#include <cmath>

namespace lol_at_home_engine {

Renderer::Renderer(SDL_Renderer* sdlRenderer, std::shared_ptr<Camera> camera)
    : sdlRenderer_(sdlRenderer), camera_(std::move(camera)) {}

void Renderer::Clear(Color color) {
  setSDLColor(color);
  SDL_RenderClear(sdlRenderer_);
}

void Renderer::Present() {
  SDL_RenderPresent(sdlRenderer_);
}

void Renderer::DrawCircle(Vector2 worldPos, double radius, Color color) {
  Vector2 screenPos = camera_->WorldToScreen(worldPos, screenSize_);
  double screenRadius = radius / camera_->GetZoom();

  setSDLColor(color);

  int centerX = static_cast<int>(screenPos.x);
  int centerY = static_cast<int>(screenPos.y);
  int radiusPx = static_cast<int>(screenRadius);

  for (int offsetY = -radiusPx; offsetY <= radiusPx; offsetY++) {
    int halfWidth = static_cast<int>(
        std::sqrt((radiusPx * radiusPx) - (offsetY * offsetY)));

    SDL_RenderLine(sdlRenderer_, centerX - halfWidth, centerY + offsetY,
                   centerX + halfWidth, centerY + offsetY);
  }
}

void Renderer::DrawRect(Vector2 worldPos, Vector2 size, Color color) {
  Vector2 screenPos = camera_->WorldToScreen(worldPos, screenSize_);
  Vector2 screenSize = size / camera_->GetZoom();

  setSDLColor(color);

  SDL_FRect rect{
      static_cast<float>(screenPos.x), static_cast<float>(screenPos.y),
      static_cast<float>(screenSize.x), static_cast<float>(screenSize.y)};
  SDL_RenderFillRect(sdlRenderer_, &rect);
}

void Renderer::DrawLine(Vector2 worldPosStart,
                        Vector2 worldPosEnd,
                        Color color) {
  Vector2 screenStart = camera_->WorldToScreen(worldPosStart, screenSize_);
  Vector2 screenEnd = camera_->WorldToScreen(worldPosEnd, screenSize_);

  setSDLColor(color);
  SDL_RenderLine(sdlRenderer_, static_cast<float>(screenStart.x),
                 static_cast<float>(screenStart.y),
                 static_cast<float>(screenEnd.x),
                 static_cast<float>(screenEnd.y));
}

void Renderer::DrawRectUI(Vector2 screenPos, Vector2 size, Color color) {
  setSDLColor(color);

  SDL_FRect rect{static_cast<float>(screenPos.x),
                 static_cast<float>(screenPos.y), static_cast<float>(size.x),
                 static_cast<float>(size.y)};
  SDL_RenderFillRect(sdlRenderer_, &rect);
}

void Renderer::UpdateScreenSize(int width, int height) {
  screenSize_.x = static_cast<double>(width);
  screenSize_.y = static_cast<double>(height);
}

void Renderer::setSDLColor(Color color) {
  SDL_SetRenderDrawColor(sdlRenderer_, color.r, color.g, color.b, color.a);
}

}  // namespace lol_at_home_engine
