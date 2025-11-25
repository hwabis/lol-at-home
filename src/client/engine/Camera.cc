#include "Camera.h"

namespace lol_at_home_engine {

// todo make struct (typedefs) between worldpos and screenpos
auto Camera::WorldToScreen(Vector2 worldPos, Vector2 screenSize) const
    -> Vector2 {
  constexpr float half = 2.0F;

  Vector2 relativePos = worldPos - position_;
  Vector2 screenPos = relativePos / zoom_;
  screenPos.x += screenSize.x * half;
  screenPos.y += screenSize.y * half;
  return screenPos;
}

auto Camera::ScreenToWorld(Vector2 screenPos, Vector2 screenSize) const
    -> Vector2 {
  constexpr float half = 2.0F;

  Vector2 centeredScreen = screenPos;
  centeredScreen.x -= screenSize.x * half;
  centeredScreen.y -= screenSize.y * half;
  Vector2 worldPos = centeredScreen * zoom_;
  worldPos = worldPos + position_;
  return worldPos;
}

}  // namespace lol_at_home_engine
