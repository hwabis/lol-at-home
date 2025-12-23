#pragma once

#include <SDL3/SDL.h>
#include <span>
#include "Vector2.h"

namespace lol_at_home_engine {

class InputAccessor {
 public:
  void Update();

  [[nodiscard]] auto IsMouseButtonPressed(int button) const -> bool;
  [[nodiscard]] auto IsMouseButtonReleased(int button) const -> bool;
  [[nodiscard]] auto GetMousePosition() const -> Vector2;

  [[nodiscard]] auto IsKeyPressed(SDL_Keycode key) const -> bool;
  // todo iskeyreleased

 private:
  SDL_MouseButtonFlags currentMouseState_{0};
  SDL_MouseButtonFlags previousMouseState_{0};
  Vector2 mousePosition_;

  std::span<const bool> currentKeyState_;
  std::span<const bool> previousKeyState_;
};

}  // namespace lol_at_home_engine
