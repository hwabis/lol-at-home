#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include "GameAction.h"
#include "GameState.h"

namespace lol_at_home_server {

class GameStateThread {
 public:
  explicit GameStateThread(GameState gameState);
  void Start();
  void Stop();
  void HandleInput(GameAction input);

 private:
  void runAndBlockGameLoop();
  auto getAndClearQueuedActions() -> std::vector<GameAction>;
  static void broadcastDeltaGameState(const GameStateDelta&);
  static void broadcastFullGameState(
      const std::unordered_map<EntityId, std::unique_ptr<Entity>>&);

  std::jthread gameThread_;
  std::atomic<bool> isRunning_ = false;
  std::queue<GameAction> actionQueue_;
  std::mutex actionQueueMutex_;
  GameState gameState_;
};

}  // namespace lol_at_home_server
