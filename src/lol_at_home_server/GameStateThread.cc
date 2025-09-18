#include "GameStateThread.h"
#include "Config.h"

namespace lol_at_home_server {

void GameStateThread::Start() {
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(PlayerInput input) {
  std::lock_guard<std::mutex> lock(inputQueueMutex_);
  inputQueue_.push(input);
}

void GameStateThread::runGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime = std::chrono::steady_clock::now() - Config::TickInterval;

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTimeMs =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    processQueuedInputs();
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

void GameStateThread::processQueuedInputs() {
  std::vector<PlayerInput> inputsToProcess;

  {
    std::lock_guard<std::mutex> lock(inputQueueMutex_);
    while (!inputQueue_.empty()) {
      inputsToProcess.push_back(inputQueue_.front());
      inputQueue_.pop();
    }
  }

  for (const auto& input : inputsToProcess) {
    gameState_.Process(input);
  }
}

void GameStateThread::broadcastDeltaGameState() {
  // todo
}

void GameStateThread::broadcastFullGameState() {
  // todo
}

};  // namespace lol_at_home_server
