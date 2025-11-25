#pragma once

#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include "Camera.h"
#include "Input.h"
#include "Renderer.h"
#include "Scene.h"

namespace lol_at_home_engine {

struct GameConfig {
  std::string WindowTitle = "Game";
  // todo why are there two places reoslution is defined LOL
  int windowWidth = 1280;
  int windowHeight = 720;
  int targetFPS = 60;
};

class Game {
 public:
  explicit Game(GameConfig config);
  virtual ~Game();

  Game(const Game&) = delete;
  auto operator=(const Game&) -> Game& = delete;
  Game(Game&&) = delete;
  auto operator=(Game&&) -> Game& = delete;

  void Run();

  [[nodiscard]] auto GetCamera() -> std::shared_ptr<Camera> { return camera_; }
  [[nodiscard]] auto GetInput() -> Input& { return input_; }
  [[nodiscard]] auto GetRenderer() -> Renderer& { return *renderer_; }

 protected:
  virtual auto CreateRootScene() -> std::unique_ptr<Scene> = 0;

 private:
  void initSDL();
  void cleanupSDL();
  void gameLoop();

  GameConfig config_;
  SDL_Window* window_{nullptr};
  SDL_Renderer* sdlRenderer_{nullptr};

  std::shared_ptr<Camera> camera_;
  Input input_;
  std::unique_ptr<Renderer> renderer_;
  std::unique_ptr<Scene> currentScene_;

  bool running_{false};
};

}  // namespace lol_at_home_engine
