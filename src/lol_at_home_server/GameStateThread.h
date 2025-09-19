#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "GameState.h"
#include "PlayerInput.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  void Start();
  void Stop();
  void HandleInput(PlayerInput input);

 private:
  void runAndBlockGameLoop();
  auto getAndClearQueuedInputs() -> std::vector<PlayerInput>;
  void broadcastDeltaGameState();
  void broadcastFullGameState();

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<PlayerInput> inputQueue_;
  std::mutex inputQueueMutex_;
  GameState gameState_;
};

}  // namespace lol_at_home_server
