#pragma once

#include <variant>
#include "primitives/Color.h"
#include "primitives/Vector2.h"

namespace lol_at_home_engine {

// todo sprite

struct CircleData {
  float radius{};
  Color color{};
};

struct RectData {
  Vector2 size;
  Color color;
};

using RenderableData = std::variant<CircleData, RectData>;

class IWorldRenderable {
 public:
  virtual ~IWorldRenderable();
  IWorldRenderable(const IWorldRenderable&) = delete;
  auto operator=(const IWorldRenderable&) -> IWorldRenderable& = delete;
  IWorldRenderable(IWorldRenderable&& other) = default;
  auto operator=(IWorldRenderable&&) -> IWorldRenderable& = default;

  virtual auto GetRenderableData() -> RenderableData = 0;
};

}  // namespace lol_at_home_engine
