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
  explicit GameStateThread(GameState gameState);
  void Start();
  void Stop();
  void HandleInput(PlayerInput input);

 private:
  void runAndBlockGameLoop();
  auto getAndClearQueuedInputs() -> std::vector<PlayerInput>;
  static void broadcastDeltaGameState(const std::vector<GameStateDelta>&);
  static void broadcastFullGameState(
      const std::unordered_map<EntityId, std::unique_ptr<Entity>>&);

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<PlayerInput> inputQueue_;
  std::mutex inputQueueMutex_;
  GameState gameState_;
};

}  // namespace lol_at_home_server
