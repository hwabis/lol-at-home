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

void GameStateThread::HandlePlayerInput(GameInput input) {
  std::lock_guard<std::mutex> lock(inputQueueMutex_);
  inputQueue_.push(input);
}

void GameStateThread::runGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();

    processQueuedInputs();
    updateGameState();

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
  std::vector<GameInput> inputsToProcess;

  {
    std::lock_guard<std::mutex> lock(inputQueueMutex_);
    while (!inputQueue_.empty()) {
      inputsToProcess.push_back(inputQueue_.front());
      inputQueue_.pop();
    }
  }

  for (const auto& input : inputsToProcess) {
    switch (input.Type) {
      // todo this does stuff in our game state class
    }
  }
}

void GameStateThread::updateGameState() {
  // todo
}

void GameStateThread::broadcastDeltaGameState() {
  // todo
}

void GameStateThread::broadcastFullGameState() {
  // todo
}

};  // namespace lol_at_home_server
