#include "sync_queue.h"
#include "thread_pool.h"
#include <chrono>
#include <cstdio>
#include <iostream>
#include <thread>

void TestThdPool() {
  ThreadPool pool;
  pool.Start(2);

  std::thread thd1([&pool] {
    for (int i = 0; i < 10; i++) {
      auto thdId = this_thread::get_id();
      pool.AddTask([thdId] {
        std::cout << "Sync Layer Thread1 ID: " << thdId << std::endl;
      });
    }
  });

  std::thread thd2([&pool] {
    for (int i = 0; i < 10; ++i) {
      auto thdId = this_thread::get_id();
      pool.AddTask([thdId] {
        std::cout << "Sync Layer Thread2 ID: " << thdId << std::endl;
      });
    }
  });

  this_thread::sleep_for(std::chrono::seconds(2));
  getchar();
  pool.Stop();
  thd1.join();
  thd2.join();
};

int main(int argc, char *argv[]) {
  TestThdPool();
  return 0;
}
