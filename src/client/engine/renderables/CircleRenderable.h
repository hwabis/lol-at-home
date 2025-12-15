#pragma once

#include <cmath>
#include "IWorldRenderable.h"
#include "primitives/Color.h"
#include "primitives/Transform.h"

namespace lol_at_home_engine {

class CircleRenderable : public IWorldRenderable {
 public:
  CircleRenderable(Transform transform, float radius, Color color)
      : transform_(transform), radius_(radius), color_(color) {}

  void Render(SDL_Renderer* renderer, const Camera& camera) override {
    // todo needs to use camera

    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, color_.a);

    const float scaledRadius = radius_ * transform_.scale;

    const int intRadius = static_cast<int>(std::ceil(scaledRadius));

    const float baseY = transform_.position.y;
    const float baseX = transform_.position.x;

    for (int yDraw = -intRadius; yDraw <= intRadius; ++yDraw) {
      const auto yFloat = static_cast<float>(yDraw);
      const float deltaX =
          std::sqrt((scaledRadius * scaledRadius) - (yFloat * yFloat));

      SDL_RenderLine(renderer, baseX - deltaX, baseY + yFloat, baseX + deltaX,
                     baseY + yFloat);
    }
  }

 private:
  Transform transform_;
  float radius_;
  Color color_;
};

}  // namespace lol_at_home_engine
