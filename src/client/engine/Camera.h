#pragma once

#include "Vector2.h"

namespace lah::engine {

class Camera {
 public:
  explicit Camera(Vector2 viewportSize) : viewportSize_(viewportSize) {
    RecalculateView(viewportSize);
  }

  [[nodiscard]] auto WorldToScreen(Vector2 worldPos) const -> Vector2;
  [[nodiscard]] auto ScreenToWorld(Vector2 screenPos) const -> Vector2;

  void SetPosition(Vector2 pos) { worldPosition_ = pos; }
  [[nodiscard]] auto GetPosition() const -> Vector2 { return worldPosition_; }

  void RecalculateView(Vector2 viewportSize);

 private:
  Vector2 worldPosition_{};
  Vector2 viewportSize_{};

  // todo for now zoom is only dynamically changed internally to see the same
  // amount of the world
  float zoom_ = 1.0F;

  static constexpr float referenceHeight = 720.0F;
};

}  // namespace lah::engine
