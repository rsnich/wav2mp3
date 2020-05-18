#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class thread_pool {
public:
  static constexpr size_t task_limit_default = 10000;
  using task_t = std::function<void()>;

private:
  class task_queue {
  public:
    task_queue();
    ~task_queue();
    bool try_push(task_t &&task);
    void push(task_t &&task);
    thread_pool::task_t try_pop();
    thread_pool::task_t pop();
    void set_task_limit(size_t task_limit);
    bool is_empty();
    void exit();

  private:
    std::queue<task_t> m_qtasks;
    std::mutex m_tq_mutex;
    std::condition_variable m_tq_cv_pop;
    std::condition_variable m_tq_cv_push;
    std::atomic_bool m_do_exit;
    std::atomic_size_t m_task_limit;
  };

public:
  thread_pool(size_t thread_num = std::thread::hardware_concurrency(),
              size_t task_limit = task_limit_default);
  ~thread_pool();
  void enqueue_task(task_t &&task);
  void exit();
  void wait_finished();

private:
  std::mutex m_finished_mutex;
  std::vector<std::thread> m_threads;
  std::vector<task_queue> m_thread_queues;
  const std::atomic_size_t m_thread_num;
  std::atomic_bool m_do_thread_stop;
  std::atomic_uint m_task_count;
  std::condition_variable m_finished_cv;
};