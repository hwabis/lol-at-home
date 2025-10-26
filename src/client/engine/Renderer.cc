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

  int cx = static_cast<int>(screenPos.X);
  int cy = static_cast<int>(screenPos.Y);
  int r = static_cast<int>(screenRadius);

  for (int y = -r; y <= r; y++) {
    int x = static_cast<int>(std::sqrt(r * r - y * y));
    SDL_RenderLine(sdlRenderer_, cx - x, cy + y, cx + x, cy + y);
  }
}

void Renderer::DrawRect(Vector2 worldPos, Vector2 size, Color color) {
  Vector2 screenPos = camera_->WorldToScreen(worldPos, screenSize_);
  Vector2 screenSize = size / camera_->GetZoom();

  setSDLColor(color);

  SDL_FRect rect{
      static_cast<float>(screenPos.X), static_cast<float>(screenPos.Y),
      static_cast<float>(screenSize.X), static_cast<float>(screenSize.Y)};
  SDL_RenderFillRect(sdlRenderer_, &rect);
}

void Renderer::DrawLine(Vector2 worldPosStart,
                        Vector2 worldPosEnd,
                        Color color) {
  Vector2 screenStart = camera_->WorldToScreen(worldPosStart, screenSize_);
  Vector2 screenEnd = camera_->WorldToScreen(worldPosEnd, screenSize_);

  setSDLColor(color);
  SDL_RenderLine(sdlRenderer_, static_cast<float>(screenStart.X),
                 static_cast<float>(screenStart.Y),
                 static_cast<float>(screenEnd.X),
                 static_cast<float>(screenEnd.Y));
}

void Renderer::DrawRectUI(Vector2 screenPos, Vector2 size, Color color) {
  setSDLColor(color);

  SDL_FRect rect{static_cast<float>(screenPos.X),
                 static_cast<float>(screenPos.Y), static_cast<float>(size.X),
                 static_cast<float>(size.Y)};
  SDL_RenderFillRect(sdlRenderer_, &rect);
}

void Renderer::UpdateScreenSize(int width, int height) {
  screenSize_ = {.X = static_cast<double>(width),
                 .Y = static_cast<double>(height)};
}

void Renderer::setSDLColor(Color color) {
  SDL_SetRenderDrawColor(sdlRenderer_, color.r, color.g, color.b, color.a);
}

}  // namespace lol_at_home_engine
