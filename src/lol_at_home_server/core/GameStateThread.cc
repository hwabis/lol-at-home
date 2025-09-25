#include "core/GameStateThread.h"

namespace lol_at_home_server {

GameStateThread::GameStateThread(GameState startGameState,
                                 GameStateThreadConfig config)
    : gameState_(std::move(startGameState)), config_(config) {}

void GameStateThread::Start() {
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runAndBlockGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(GameActionVariant input) {
  std::lock_guard<std::mutex> lock(actionQueueMutex_);
  actionQueue_.push(input);
}

void GameStateThread::runAndBlockGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime =
      std::chrono::steady_clock::now() - config_.UpdateInterval;

  while (isRunning_) {
    auto frameStart = std::chrono::steady_clock::now();
    auto deltaTimeMs =
        std::chrono::duration<double, std::milli>(frameStart - lastFrameTime)
            .count();
    lastFrameTime = frameStart;

    auto actions = getAndClearQueuedActions();
    auto delta = gameState_.ProcessActionsAndUpdate(actions, deltaTimeMs);
    broadcastDeltaGameState(delta);

    if (frameStart - lastFullStateBroadcast >=
        config_.FullStateBroadcastInterval) {
      broadcastFullGameState(gameState_.Registry);
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < config_.UpdateInterval) {
      std::this_thread::sleep_for(config_.UpdateInterval - elapsed);
    }
  }
}

auto GameStateThread::getAndClearQueuedActions()
    -> std::vector<GameActionVariant> {
  std::vector<GameActionVariant> inputs;

  {
    std::lock_guard<std::mutex> lock(actionQueueMutex_);
    while (!actionQueue_.empty()) {
      inputs.push_back(actionQueue_.front());
      actionQueue_.pop();
    }
  }

  return inputs;
}

void GameStateThread::broadcastDeltaGameState(const GameStateDelta&) {
  // todo probably only broadcast dirty entities
}

void GameStateThread::broadcastFullGameState(const entt::registry&) {
  // todo probably only broadcast entities with position component
}

};  // namespace lol_at_home_server
