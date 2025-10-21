#pragma once

#include "Vector2.h"

namespace lol_at_home_engine {

class Camera {
 public:
  Camera() = default;

  [[nodiscard]] auto WorldToScreen(Vector2 worldPos, Vector2 screenSize) const
      -> Vector2;
  [[nodiscard]] auto ScreenToWorld(Vector2 screenPos, Vector2 screenSize) const
      -> Vector2;

  void SetPosition(Vector2 pos) { position_ = pos; }
  [[nodiscard]] auto GetPosition() const -> Vector2 { return position_; }

  void SetZoom(float zoom);
  [[nodiscard]] auto GetZoom() const -> float { return zoom_; }

  void SetZoomLimits(float min, float max) {
    minZoom_ = min;
    maxZoom_ = max;
  }

 private:
  Vector2 position_{.X = 0.0, .Y = 0.0};
  float zoom_{1.0F};
  float minZoom_{0.5F};
  float maxZoom_{2.0F};
};

}  // namespace lol_at_home_engine
