#include "Input.h"

namespace lol_at_home_engine {

void Input::Update() {
  previousMouseState_ = currentMouseState_;

  float mx = 0.0F;
  float my = 0.0F;
  currentMouseState_ = SDL_GetMouseState(&mx, &my);
  mousePosition_ = {.X = static_cast<double>(mx), .Y = static_cast<double>(my)};

  int numKeys = 0;
  const bool* state = SDL_GetKeyboardState(&numKeys);
  currentKeyState_ = std::span<const bool>(state, numKeys);
}

auto Input::IsMouseButtonPressed(int button) const -> bool {
  SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
  return (currentMouseState_ & mask) != 0 && (previousMouseState_ & mask) == 0;
}

auto Input::IsMouseButtonReleased(int button) const -> bool {
  SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
  return (currentMouseState_ & mask) == 0 && (previousMouseState_ & mask) != 0;
}

auto Input::GetMousePosition() const -> Vector2 {
  return mousePosition_;
}

auto Input::IsKeyPressed(SDL_Keycode key) const -> bool {
  if (currentKeyState_.empty()) {
    return false;
  }

  SDL_Scancode scancode = SDL_GetScancodeFromKey(key, nullptr);
  return currentKeyState_[scancode];
}

auto Input::IsKeyReleased(SDL_Keycode key) const -> bool {
  // Would need previous keyboard state tracking
  return false;
}

}  // namespace lol_at_home_engine
