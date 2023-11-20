#pragma once
#include <condition_variable>
#include <cstddef>
#include <iostream>
#include <list>
#include <mutex>
#include <ostream>
#include <thread>

using namespace std;

template <typename T> class SyncQueue {
public:
  explicit SyncQueue(int maxSize) : m_maxSize{maxSize}, m_needStop{false} {
    std::cout << "SyncQueue Construct" << std::endl;
  }
  ~SyncQueue() = default;

  bool Empty() {
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_queue.empty();
  }

  bool Full() {
    // need to acquire a mutex to check if the m_queue is full
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_queue.size() == m_maxSize;
  }

  size_t Size() {
    std::lock_guard<std::mutex> lg(m_mutex);
    return m_queue.size();
  }

  void Stop() {
    {
      std::lock_guard<std::mutex> sl(m_mutex);
      m_needStop = true;
    }
    m_notFull.notify_all();
    m_notEmpty.notify_all();
  }

  void Put(const T &x) { Add(x); }

  void Put(T &&x) { Add(std::forward<T>(x)); }

  void Take(T &t) {
    std::unique_lock<std::mutex> ul(m_mutex);

    m_notEmpty.wait(ul, [this] {
      return m_needStop || [this] { return !m_queue.empty(); }();
    });

    if (m_needStop)
      return;

    // TODO: for only one task, we can use pop_front
    t = m_queue.front();
    m_queue.pop_front();
    m_notFull.notify_one();
  }

  void Take(std::list<T> &list) {

    std::unique_lock<std::mutex> ul(m_mutex);

    m_notEmpty.wait(ul, [this] {
      return m_needStop || [this] { return !m_queue.empty(); }();
    });

    if (m_needStop)
      return;

    // TODO: for a list, we use std::move to get whole list efficiently
    list = std::move(m_queue);
    m_notFull.notify_one();
  }

private:
  template <typename F> void Add(F &&x) {
    std::unique_lock<std::mutex> ul(m_mutex); // must use unique_lock!!!

    m_notFull.wait(ul, [this] {
      return m_needStop || [this] { return m_queue.size() < m_maxSize; }();
    });

    if (m_needStop)
      return;

    m_queue.push_back(x); // TODO:why use std::forward here?
    m_notEmpty.notify_one();
  }

private:
  std::list<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_notEmpty;
  std::condition_variable m_notFull;

  int m_maxSize;

  bool m_needStop;
};
