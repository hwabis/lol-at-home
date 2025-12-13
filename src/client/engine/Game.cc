#include "Game.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <utility>

namespace lol_at_home_engine {

Game::Game(GameConfig config) : config_(std::move(config)) {}

Game::~Game() {
  cleanupSDL();
}

void Game::Run(
    const std::function<std::unique_ptr<Scene>(SDL_Renderer*)>& sceneFactory) {
  initSDL();

  scene_ = sceneFactory(sdlRenderer_);

  running_ = true;
  gameLoop();
}

void Game::initSDL() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error(std::string("SDL_Init failed: ") + SDL_GetError());
    throw std::runtime_error("SDL initialization failed");
  }

  window_ = SDL_CreateWindow(config_.WindowTitle.c_str(), config_.windowWidth,
                             config_.windowHeight, 0);
  if (window_ == nullptr) {
    spdlog::error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());
    SDL_Quit();
    throw std::runtime_error("Window creation failed");
  }

  sdlRenderer_ = SDL_CreateRenderer(window_, nullptr);
  if (sdlRenderer_ == nullptr) {
    spdlog::error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());
    SDL_DestroyWindow(window_);
    SDL_Quit();
    throw std::runtime_error("Renderer creation failed");
  }
}

void Game::cleanupSDL() {
  if (sdlRenderer_ != nullptr) {
    SDL_DestroyRenderer(sdlRenderer_);
    sdlRenderer_ = nullptr;
  }

  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
}

void Game::gameLoop() {
  auto lastFrameTime = std::chrono::steady_clock::now();
  const double targetFrameTime = 1000.0 / config_.targetFPS;

  while (running_) {
    auto frameStart = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> deltaTime =
        frameStart - lastFrameTime;
    lastFrameTime = frameStart;

    scene_->Render();
    scene_->Update(deltaTime);

    if (!scene_->ShouldContinue()) {
      running_ = false;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto frameTime =
        std::chrono::duration<double, std::milli>(frameEnd - frameStart)
            .count();

    if (frameTime < targetFrameTime) {
      SDL_Delay(static_cast<uint32_t>(targetFrameTime - frameTime));
    }
  }
}

}  // namespace lol_at_home_engine
