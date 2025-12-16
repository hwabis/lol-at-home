#pragma once

#include <SDL3/SDL.h>
#include <functional>
#include <memory>
#include <string>
#include "Scene.h"

namespace lol_at_home_engine {

struct GameConfig {
  std::string windowTitle = "Game";
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

  void Run(const std::function<std::unique_ptr<
               Scene>(SDL_Renderer*, int width, int height)>& sceneFactory);

 private:
  void initSDL();
  void cleanupSDL();
  void gameLoop();

  GameConfig activeConfig_;
  SDL_Window* window_ = nullptr;
  SDL_Renderer* sdlRenderer_ = nullptr;

  std::unique_ptr<Scene> scene_;

  bool running_{false};
};

}  // namespace lol_at_home_engine
