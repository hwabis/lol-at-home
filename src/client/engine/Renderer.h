#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include "Camera.h"
#include "Vector2.h"

namespace lol_at_home_engine {

class Renderer {
 public:
  Renderer(SDL_Renderer* sdlRenderer, std::shared_ptr<Camera> camera);

  void Clear(Color color = {});
  void Present();

  void DrawCircle(Vector2 worldPos, double radius, Color color);
  void DrawRect(Vector2 worldPos, Vector2 size, Color color);
  void DrawLine(Vector2 worldPosStart, Vector2 worldPosEnd, Color color);

  void DrawRectUI(Vector2 screenPos, Vector2 size, Color color);

  [[nodiscard]] auto GetScreenSize() const -> Vector2 { return screenSize_; }
  void UpdateScreenSize(int width, int height);

 private:
  void setSDLColor(Color color);

  SDL_Renderer* sdlRenderer_;
  std::shared_ptr<Camera> camera_;
  Vector2 screenSize_{.X = 800.0, .Y = 600.0};
};

}  // namespace lol_at_home_engine
