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

  // todo should these be here? what abt sprites and stuff? scene graph? do we
  // even need a renderer class? renderer ecs system? what's going on xD
  void DrawCircle(Vector2 worldPos, double radius, Color color);
  void DrawRect(Vector2 worldPos, Vector2 size, Color color);
  void DrawLine(Vector2 worldPosStart, Vector2 worldPosEnd, Color color);

  // todo should ui even be here?
  void DrawRectUI(Vector2 screenPos, Vector2 size, Color color);

  [[nodiscard]] auto GetScreenSize() const -> Vector2 { return screenSize_; }
  void UpdateScreenSize(int width, int height);

 private:
  void setSDLColor(Color color);

  SDL_Renderer* sdlRenderer_;
  std::shared_ptr<Camera> camera_;
  // todo settable somewhere? virtual or physical resolution?
  Vector2 screenSize_{.x = 800.0, .y = 600.0};
};

}  // namespace lol_at_home_engine
