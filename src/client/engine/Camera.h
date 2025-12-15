#pragma once

#include "primitives/Vector2.h"

namespace lol_at_home_engine {

class Camera {
 public:
  Camera(int width, int height) {
    viewportSize_ = {.x = static_cast<float>(width),
                     .y = static_cast<float>(height)};
  }

  [[nodiscard]] auto WorldToScreen(Vector2 worldPos) const -> Vector2;
  [[nodiscard]] auto ScreenToWorld(Vector2 screenPos) const -> Vector2;

  void SetPosition(Vector2 pos) { worldPosition_ = pos; }
  [[nodiscard]] auto GetPosition() const -> Vector2 { return worldPosition_; }

  void RecalculateView(int width, int height);

 private:
  Vector2 worldPosition_{};
  Vector2 viewportSize_{};

  // todo for now zoom is only dynamically changed internally to see the same
  // amount of the world
  float zoom_ = 1.0F;

  static constexpr float referenceHeight = 720.0F;
};

}  // namespace lol_at_home_engine
