#ifndef THREAD_POOL_SIMPLE_H
#define THREAD_POOL_SIMPLE_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>

class ThreadPoolSimple {
public:
    explicit ThreadPoolSimple(size_t num_threads) {
        if (num_threads == 0) {
            throw std::invalid_argument("Number of threads must be at least 1");
        }
        start(num_threads);
    }

    ~ThreadPoolSimple() {
        stop();
    }

    // 递归展开参数包的辅助函数
    template <typename F, typename Tuple, size_t... Indices>
    void apply_impl(F&& f, Tuple&& tuple, std::index_sequence<Indices...>) {
        f(std::get<Indices>(std::forward<Tuple>(tuple))...);
    }
    
    // 模拟C++17的std::apply函数
    template <typename F, typename Tuple>
    void apply(F&& f, Tuple&& tuple) {
        apply_impl(std::forward<F>(f), std::forward<Tuple>(tuple),
                   std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{});
    }
    
    template <typename F, typename... Args>
    void enqueue(F&& f, Args&&... args) {
        { 
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace([this, f = std::forward<F>(f), args = std::make_tuple(std::forward<Args>(args)...)]() {
                // 执行任务
                apply(f, args);
            });
            active_tasks++;
        }
        condition.notify_one();
    }
    
    // 等待所有任务完成
    void wait() {
        std::unique_lock<std::mutex> lock(queue_mutex);
        wait_condition.wait(lock, [this] { return active_tasks == 0 && tasks.empty(); });
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::condition_variable wait_condition;
    bool stop_flag = false;
    size_t active_tasks = 0;

    void start(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] { 
                while (true) {
                    std::function<void()> task;
                    { 
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop_flag || !tasks.empty(); });
                        if (stop_flag && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                    // 任务完成，更新active_tasks并通知wait_condition
                    { 
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        active_tasks--;
                        if (active_tasks == 0 && tasks.empty()) {
                            wait_condition.notify_all();
                        }
                    }
                }
            });
        }
    }

    void stop() {
        { 
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop_flag = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

#endif // THREAD_POOL_SIMPLE_H