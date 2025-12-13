#pragma once

#include <algorithm>
#include "primitives/Vector2.h"

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

  void SetZoom(float zoom) { zoom_ = std::clamp(zoom, minZoom_, maxZoom_); };
  [[nodiscard]] auto GetZoom() const -> float { return zoom_; }

  void SetZoomLimits(float min, float max) {
    minZoom_ = min;
    maxZoom_ = max;
  }

 private:
  Vector2 position_{.x = 0.0, .y = 0.0};
  float zoom_ = 1.0F;
  static constexpr float defaultMinZoom = 0.5F;
  float minZoom_ = defaultMinZoom;
  static constexpr float defaultMaxZoom = 2.0F;
  float maxZoom_ = defaultMaxZoom;

  // todo split zoom into targetZoom and currentZoom for easing zooms or smth
};

}  // namespace lol_at_home_engine
