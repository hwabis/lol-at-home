#include "Game.h"
#include <SDL3/SDL_video.h>
#include <spdlog/spdlog.h>
#include <chrono>

namespace lah::engine {

Game::Game(const GameConfig& config) : info_(config) {}

void Game::Run(Scene&& scene) {
  initSDL();
  scene_ = std::move(scene);
  sceneLoop();
  cleanupSDL();
}

void Game::initSDL() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error(std::string("SDL_Init failed: ") + SDL_GetError());
    throw std::runtime_error("SDL initialization failed");
  }

  unsigned long long flags = 0;
  switch (info_.config.windowType) {
    case GameConfig::WindowType::Windowed:
      flags = SDL_WINDOW_RESIZABLE;
      break;
    case GameConfig::WindowType::Fullscreen:
      flags = SDL_WINDOW_FULLSCREEN;
      break;
    case GameConfig::WindowType::BorderlessFullscreen:
      flags = SDL_WINDOW_BORDERLESS;
      break;
  }

  info_.window =
      SDL_CreateWindow(info_.config.windowTitle.c_str(),
                       static_cast<int>(info_.config.windowSize.x),
                       static_cast<int>(info_.config.windowSize.y), flags);
  if (info_.window == nullptr) {
    spdlog::error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    SDL_Quit();
    throw std::runtime_error("Window creation failed");
  }

  info_.sdlRenderer = SDL_CreateRenderer(info_.window, nullptr);
  if (info_.sdlRenderer == nullptr) {
    spdlog::error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    SDL_DestroyWindow(info_.window);
    SDL_Quit();
    throw std::runtime_error("Renderer creation failed");
  }
}

void Game::cleanupSDL() {
  if (info_.sdlRenderer != nullptr) {
    SDL_DestroyRenderer(info_.sdlRenderer);
    info_.sdlRenderer = nullptr;
  }

  if (info_.window != nullptr) {
    SDL_DestroyWindow(info_.window);
    info_.window = nullptr;
  }

  SDL_Quit();
}

void Game::sceneLoop() {
  auto lastFrameTime = std::chrono::steady_clock::now();
  const double targetFrameTime = 1000.0 / info_.config.targetFPS;
  bool running_ = true;

  while (running_) {
    auto frameStart = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> deltaTime =
        frameStart - lastFrameTime;
    lastFrameTime = frameStart;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          running_ = false;
          break;
        case SDL_EVENT_WINDOW_RESIZED:
          info_.config.windowSize.x = static_cast<float>(event.window.data1);
          info_.config.windowSize.y = static_cast<float>(event.window.data2);
          info_.camera.RecalculateView({
              .x = info_.config.windowSize.x,
              .y = info_.config.windowSize.y,
          });
          break;
        default:
          break;
      }
    }

    info_.input.Update();
    scene_.Cycle(info_, deltaTime);

    auto frameEnd = std::chrono::steady_clock::now();
    auto frameTime =
        std::chrono::duration<double, std::milli>(frameEnd - frameStart)
            .count();

    if (frameTime < targetFrameTime) {
      SDL_Delay(static_cast<uint32_t>(targetFrameTime - frameTime));
    }
  }
}

}  // namespace lah::engine
