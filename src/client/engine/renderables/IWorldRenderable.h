#pragma once

#include <SDL3/SDL.h>
#include "Camera.h"

namespace lol_at_home_engine {

class IWorldRenderable {
 public:
  IWorldRenderable() = default;
  virtual ~IWorldRenderable() = default;
  IWorldRenderable(const IWorldRenderable&) = delete;
  auto operator=(const IWorldRenderable&) -> IWorldRenderable& = delete;
  IWorldRenderable(IWorldRenderable&& other) = default;
  auto operator=(IWorldRenderable&&) -> IWorldRenderable& = default;

  virtual void Render(SDL_Renderer* renderer, const Camera& camera) = 0;
};

}  // namespace lol_at_home_engine
