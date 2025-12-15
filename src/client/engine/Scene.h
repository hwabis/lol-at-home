#pragma once

#include <chrono>
#include <memory>
#include <vector>
#include "Camera.h"
#include "GameObject.h"
#include "InputAccessor.h"

namespace lol_at_home_engine {

class Renderer;

class Scene {
 public:
  Scene(SDL_Renderer* renderer, int width, int height);
  virtual ~Scene() = default;
  Scene(const Scene&) = default;
  auto operator=(const Scene&) -> Scene& = default;
  Scene(Scene&&) = default;
  auto operator=(Scene&&) -> Scene& = default;

  // todo
  /*
    virtual void OnStart() {}
    virtual void OnEnd() {}
  */

  void Render();
  void Update(std::chrono::duration<double, std::milli> deltaTime);
  [[nodiscard]] auto ShouldContinue() -> bool {
    return continue_ && ShouldContinueImpl();
  }

 protected:
  void AddObject(std::unique_ptr<GameObject> obj) {
    objects_.push_back(std::move(obj));
  }

  [[nodiscard]] virtual auto ShouldContinueImpl() const -> bool { return true; }

  [[nodiscard]] auto GetCamera() const -> Camera { return camera_; }
  [[nodiscard]] auto GetInput() const -> InputAccessor { return input_; }

 private:
  SDL_Renderer* renderer_{};

  std::vector<std::unique_ptr<GameObject>> objects_;
  Camera camera_;
  InputAccessor input_;
  bool continue_ = true;
};

}  // namespace lol_at_home_engine
