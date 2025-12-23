#include "InputAccessor.h"

namespace lol_at_home_engine {

void InputAccessor::Update() {
  previousMouseState_ = currentMouseState_;
  currentMouseState_ = SDL_GetMouseState(&mousePosition_.x, &mousePosition_.y);

  int numKeys = 0;
  const bool* state = SDL_GetKeyboardState(&numKeys);
  currentKeyState_ = std::span<const bool>(state, numKeys);
}

auto InputAccessor::IsMouseButtonPressed(int button) const -> bool {
  SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
  return (currentMouseState_ & mask) != 0 && (previousMouseState_ & mask) == 0;
}

auto InputAccessor::IsMouseButtonReleased(int button) const -> bool {
  SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
  return (currentMouseState_ & mask) == 0 && (previousMouseState_ & mask) != 0;
}

auto InputAccessor::GetMousePosition() const -> Vector2 {
  return mousePosition_;
}

auto InputAccessor::IsKeyPressed(SDL_Keycode key) const -> bool {
  if (currentKeyState_.empty()) {
    return false;
  }

  SDL_Scancode scancode = SDL_GetScancodeFromKey(key, nullptr);
  return currentKeyState_[scancode];
}

}  // namespace lol_at_home_engine
