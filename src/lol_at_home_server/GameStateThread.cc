#include "GameStateThread.h"

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
  // todo all these configurable as hz
  constexpr auto tickInterval = std::chrono::milliseconds(33);
  constexpr auto fullStateBroadcastInterval = std::chrono::seconds(5);

  auto lastFullStateBroadcast = std::chrono::steady_clock::now();

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();

    processQueuedInputs();
    updateGameState();

    broadcastDeltaGameState();

    if (frameStart - lastFullStateBroadcast >= fullStateBroadcastInterval) {
      broadcastFullGameState();
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < tickInterval) {
      std::this_thread::sleep_for(tickInterval - elapsed);
    }
  }
}

void GameStateThread::processQueuedInputs() {
  std::lock_guard<std::mutex> lock(inputQueueMutex_);
  while (!inputQueue_.empty()) {
    GameInput input = inputQueue_.front();
    inputQueue_.pop();

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
