#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <chrono>

class AdvancedThreadPool {
public:
    AdvancedThreadPool(size_t threads) : stop(false) {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this] { 
                            return this->stop || !this->tasks.empty(); 
                        });
                        if (this->stop && this->tasks.empty()) return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    
    // Day 2 Feature: Templates & Future returns
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::result_of<F(Args...)>::type> {
        
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) throw std::runtime_error("Enqueue on stopped ThreadPool");
            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~AdvancedThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) worker.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    AdvancedThreadPool pool(4);
    
    // Enqueue tasks that return actual dynamic data types back to the main thread
    auto task1 = pool.enqueue([](int id) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return "TrustRep Node: Verified zero-knowledge proof for User ID " + std::to_string(id);
    }, 1001);

    auto task2 = pool.enqueue([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        return 42.00; // Returns a double value representing analytical inference certainty
    });

    // Main thread blocks cleanly until the future results are available
    std::cout << "[Day 2 Result 1] " << task1.get() << std::endl;
    std::cout << "[Day 2 Result 2] Inference Certainty Score: " << task2.get() << "%" << std::endl;

    return 0;
}