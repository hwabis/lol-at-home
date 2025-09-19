#include "GameStateThread.h"
#include "Config.h"

namespace lol_at_home_server {

void GameStateThread::Start() {
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runAndBlockGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(PlayerInput input) {
  std::lock_guard<std::mutex> lock(inputQueueMutex_);
  inputQueue_.push(input);
}

void GameStateThread::runAndBlockGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime = std::chrono::steady_clock::now() - Config::TickInterval;

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTimeMs =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    auto inputs = getAndClearQueuedInputs();
    for (const auto& input : inputs) {
      gameState_.Process(input);
    }
    gameState_.Tick(deltaTimeMs);
    broadcastDeltaGameState();

    if (frameStart - lastFullStateBroadcast >=
        Config::FullStateBroadcastInterval) {
      broadcastFullGameState();
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < Config::TickInterval) {
      std::this_thread::sleep_for(Config::TickInterval - elapsed);
    }
  }
}

auto GameStateThread::getAndClearQueuedInputs() -> std::vector<PlayerInput> {
  std::vector<PlayerInput> inputs;

  {
    std::lock_guard<std::mutex> lock(inputQueueMutex_);
    while (!inputQueue_.empty()) {
      inputs.push_back(inputQueue_.front());
      inputQueue_.pop();
    }
  }

  return inputs;
}

void GameStateThread::broadcastDeltaGameState() {
  // todo
}

void GameStateThread::broadcastFullGameState() {
  // todo
}

};  // namespace lol_at_home_server
