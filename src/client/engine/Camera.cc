#include "Camera.h"

namespace lah::engine {

// todo make struct (typedefs) between worldpos and screenpos
auto Camera::WorldToScreen(Vector2 worldPos) const -> Vector2 {
  constexpr float half = 0.5F;

  Vector2 relativePos = worldPos - worldPosition_;
  Vector2 screenPos = relativePos / zoom_;

  screenPos.x += viewportSize_.x * half;
  screenPos.y += viewportSize_.y * half;
  return screenPos;
}

auto Camera::ScreenToWorld(Vector2 screenPos) const -> Vector2 {
  constexpr float half = 0.5F;

  Vector2 centeredScreen = screenPos;
  centeredScreen.x -= viewportSize_.x * half;
  centeredScreen.y -= viewportSize_.y * half;
  Vector2 worldPos = centeredScreen * zoom_;
  worldPos = worldPos + worldPosition_;
  return worldPos;
}

void Camera::RecalculateView(Vector2 viewportSize) {
  viewportSize_ = viewportSize;
  zoom_ = viewportSize.y / referenceHeight;
}

}  // namespace lah::engine
