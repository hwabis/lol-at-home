#pragma once

#include <mutex>
#include <optional>
#include <queue>

namespace lol_at_home_server {

template <typename T>
class ThreadSafeQueue {
 public:
  virtual ~ThreadSafeQueue() = default;
  ThreadSafeQueue(const ThreadSafeQueue&) = delete;
  auto operator=(const ThreadSafeQueue&) -> ThreadSafeQueue& = delete;
  ThreadSafeQueue(const ThreadSafeQueue&&) = delete;
  auto operator=(const ThreadSafeQueue&&) -> ThreadSafeQueue& = delete;

  void Push(const T& value) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(value);
  }

  auto PopAll() -> std::vector<T> {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<T> allItems;
    allItems.reserve(queue_.size());

    while (!queue_.empty()) {
      allItems.push_back(std::move(queue_.front()));
      queue_.pop();
    }

    return allItems;
  }

  [[nodiscard]] auto Empty() const -> bool {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  [[nodiscard]] auto Size() const -> size_t {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  std::queue<T> queue_;
  mutable std::mutex mutex_;
};

}  // namespace lol_at_home_server
