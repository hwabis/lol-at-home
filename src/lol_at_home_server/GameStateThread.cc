#include "GameStateThread.h"
#include "Config.h"

namespace lol_at_home_server {

GameStateThread::GameStateThread(GameState startGameState)
    : gameState_(std::move(startGameState)) {}

void GameStateThread::Start() {
  isRunning_ = true;
  gameThread_ = std::jthread([this] { runAndBlockGameLoop(); });
}

void GameStateThread::Stop() {
  isRunning_ = false;
}

void GameStateThread::HandleInput(GameAction input) {
  std::lock_guard<std::mutex> lock(actionQueueMutex_);
  actionQueue_.push(input);
}

void GameStateThread::runAndBlockGameLoop() {
  auto lastFullStateBroadcast = std::chrono::steady_clock::now();
  auto lastFrameTime =
      std::chrono::steady_clock::now() - Config::UpdateInterval;

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
        Config::FullStateBroadcastInterval) {
      broadcastFullGameState(gameState_.GetFullGameState());
      lastFullStateBroadcast = frameStart;
    }

    auto frameEnd = std::chrono::steady_clock::now();
    auto elapsed = frameEnd - frameStart;
    if (elapsed < Config::UpdateInterval) {
      std::this_thread::sleep_for(Config::UpdateInterval - elapsed);
    }
  }
}

auto GameStateThread::getAndClearQueuedActions() -> std::vector<GameAction> {
  std::vector<GameAction> inputs;

  {
    std::lock_guard<std::mutex> lock(actionQueueMutex_);
    while (!actionQueue_.empty()) {
      inputs.push_back(actionQueue_.front());
      actionQueue_.pop();
    }
  }

  return inputs;
}

void GameStateThread::broadcastDeltaGameState(const GameStateDelta& delta) {
  // todo
}

void GameStateThread::broadcastFullGameState(
    const std::unordered_map<EntityId, std::unique_ptr<Entity>>& gameState) {
  // todo
}

};  // namespace lol_at_home_server
