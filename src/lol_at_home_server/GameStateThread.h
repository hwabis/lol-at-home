#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "GameInput.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  void Start();
  void Stop();
  void HandlePlayerInput(GameInput input);

 private:
  void runGameLoop();
  void processQueuedInputs();
  void updateGameState();
  void broadcastDeltaGameState();
  void broadcastFullGameState();

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<GameInput> inputQueue_;
  std::mutex inputQueueMutex_;
};

}  // namespace lol_at_home_server
