#include "Camera.h"
#include <algorithm>

namespace lol_at_home_engine {

auto Camera::WorldToScreen(Vector2 worldPos, Vector2 screenSize) const
    -> Vector2 {
  Vector2 relativePos = worldPos - position_;
  Vector2 screenPos = relativePos / zoom_;
  screenPos.X += screenSize.X / 2.0;
  screenPos.Y += screenSize.Y / 2.0;
  return screenPos;
}

auto Camera::ScreenToWorld(Vector2 screenPos, Vector2 screenSize) const
    -> Vector2 {
  Vector2 centeredScreen = screenPos;
  centeredScreen.X -= screenSize.X / 2.0;
  centeredScreen.Y -= screenSize.Y / 2.0;
  Vector2 worldPos = centeredScreen * zoom_;
  worldPos = worldPos + position_;
  return worldPos;
}

void Camera::SetZoom(float zoom) {
  zoom_ = std::clamp(zoom, minZoom_, maxZoom_);
}

}  // namespace lol_at_home_engine
