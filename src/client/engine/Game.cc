#include "Game.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <utility>

namespace lol_at_home_engine {

Game::Game(GameConfig config) : config_(std::move(config)) {
  camera_ = std::make_shared<Camera>();
}

Game::~Game() {
  cleanupSDL();
}

void Game::Run() {
  initSDL();

  currentScene_ = CreateRootScene();

  currentScene_->OnStart();

  running_ = true;
  gameLoop();

  currentScene_->OnEnd();
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

  renderer_ = std::make_unique<Renderer>(sdlRenderer_, camera_);
  renderer_->UpdateScreenSize(config_.windowWidth, config_.windowHeight);

  spdlog::info("SDL initialized successfully");
}

void Game::cleanupSDL() {
  renderer_.reset();

  if (sdlRenderer_ != nullptr) {
    SDL_DestroyRenderer(sdlRenderer_);
    sdlRenderer_ = nullptr;
  }

  if (window_ != nullptr) {
    SDL_DestroyWindow(window_);
    window_ = nullptr;
  }

  SDL_Quit();
  spdlog::info("SDL cleaned up");
}

void Game::gameLoop() {
  auto lastFrameTime = std::chrono::steady_clock::now();
  const double targetFrameTime = 1000.0 / config_.targetFPS;

  while (running_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTime =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    input_.Update();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        running_ = false;
      }

      if (event.type == SDL_EVENT_WINDOW_RESIZED) {
        renderer_->UpdateScreenSize(event.window.data1, event.window.data2);
      }
    }

    currentScene_->Update(deltaTime);

    if (!currentScene_->ShouldContinue()) {
      running_ = false;
    }

    renderer_->Clear({.r = 30, .g = 30, .b = 40, .a = 255});
    if (currentScene_ != nullptr) {
      currentScene_->Render(*renderer_);
    }
    renderer_->Present();

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
