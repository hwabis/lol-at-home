#pragma once

namespace lol_at_home_engine {

class Renderer;

class Scene {
 public:
  virtual ~Scene() = default;
  Scene(const Scene&) = default;
  auto operator=(const Scene&) -> Scene& = default;
  Scene(Scene&&) = default;
  auto operator=(Scene&&) -> Scene& = default;

  virtual void OnStart() {}
  virtual void Update(double deltaTime) = 0;
  virtual void Render(Renderer& renderer) = 0;
  virtual void OnEnd() {}
  [[nodiscard]] virtual auto ShouldContinue() -> bool { return true; }

 protected:
  Scene() = default;
};

}  // namespace lol_at_home_engine
