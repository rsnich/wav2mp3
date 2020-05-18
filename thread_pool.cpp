#include "thread_pool.hpp"

/**
 * thread_pool::task_queue implementation
 */

thread_pool::task_queue::task_queue()
    : m_do_exit(false), m_task_limit(task_limit_default) {}

void thread_pool::task_queue::set_task_limit(size_t task_limit) {
  m_task_limit = task_limit;
}

thread_pool::task_queue::~task_queue() { exit(); }

bool thread_pool::task_queue::try_push(thread_pool::task_t &&task) {
  {
    std::unique_lock<std::mutex> lk(m_tq_mutex);
    if (m_qtasks.size() >= m_task_limit) {
      return false;
    }
    m_qtasks.push(std::move(task));
  }
  m_tq_cv_pop.notify_one();
  return true;
}

void thread_pool::task_queue::push(thread_pool::task_t &&task) {
  std::unique_lock<std::mutex> lk(m_tq_mutex);
  while (m_qtasks.size() >= m_task_limit) {
    m_tq_cv_push.wait(lk);
  }

  m_qtasks.push(std::move(task));
  m_tq_cv_pop.notify_one();
}

thread_pool::task_t thread_pool::task_queue::try_pop() {
  std::unique_lock<std::mutex> lk(m_tq_mutex);
  if (m_qtasks.empty()) {
    return nullptr;
  }

  thread_pool::task_t task = std::move(m_qtasks.front());
  m_qtasks.pop();
  m_tq_cv_push.notify_one();
  return task;
}

thread_pool::task_t thread_pool::task_queue::pop() {
  std::unique_lock<std::mutex> lk(m_tq_mutex);

  while (m_qtasks.empty()) {
    if (m_do_exit) {
      return nullptr;
    }
    m_tq_cv_pop.wait(lk);
  }

  thread_pool::task_t task = std::move(m_qtasks.front());
  m_qtasks.pop();
  m_tq_cv_push.notify_one();
  return task;
}

bool thread_pool::task_queue::is_empty() {
  std::unique_lock<std::mutex> lk(m_tq_mutex);
  return m_qtasks.empty();
}

void thread_pool::task_queue::exit() {
  m_do_exit = true;
  m_tq_cv_pop.notify_all();
}

/**
 * thread_pool implementation
 */

thread_pool::thread_pool(
    size_t thread_num /*= std::thread::hardware_concurrency()*/,
    size_t task_limit /*= task_limit_default*/)
    : m_thread_num(thread_num), m_thread_queues(thread_num),
      m_do_thread_stop(false), m_task_count(0) {

  for (auto &q : m_thread_queues) {
    q.set_task_limit(task_limit);
  }

  auto thread_job = [&](size_t thread_index) {
    while (!m_do_thread_stop) {
      task_t task = nullptr;
      task_queue *p_task_queue = nullptr;

      // Check all queues starting from own one
      for (size_t i = 0; i < m_thread_num; ++i) {

        p_task_queue = &m_thread_queues[(thread_index + i) % m_thread_num];
        task = p_task_queue->try_pop();
        if (task != nullptr)
          break;
      }

      // All queues are empty at the moment
      if (task == nullptr) {
        // Notify other threads the queue is empty
        m_finished_cv.notify_all();
        p_task_queue = &m_thread_queues[thread_index];
        // Wait for a new task or exit here
        task = p_task_queue->pop();
      }

      // No new tasks, do exit
      if (task == nullptr)
        break;

      // Perform the taken task
      task();
    }
  };

  // Start threads
  for (size_t i = 0; i < m_thread_num; ++i) {
    m_threads.emplace_back(thread_job, i);
  }
}

thread_pool::~thread_pool() { exit(); }

void thread_pool::enqueue_task(task_t &&task) {
  uint32_t i = m_task_count++;
  for (uint32_t n = 0; n < m_thread_num; ++n) {
    if (m_thread_queues[(i + n) % m_thread_num].try_push(std::move(task)))
      return;
  }
  m_thread_queues[i % m_thread_num].push(std::move(task));
}

void thread_pool::exit() {
  m_do_thread_stop = true;

  for (auto &q : m_thread_queues) {
    q.exit();
  }
  for (auto &t : m_threads) {
    if (t.joinable())
      t.join();
  }
}

void thread_pool::wait_finished() {
  bool all_empty = true;
  for (auto &q : m_thread_queues) {
    all_empty &= q.is_empty();
  }

  if (!all_empty) {
    std::unique_lock<std::mutex> lk(m_finished_mutex);
    m_finished_cv.wait(lk);
  }
}
