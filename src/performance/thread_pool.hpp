#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace PhotoStudio::Performance {

// Fixed-size worker pool for background jobs (export, heavy filters).
class ThreadPool {
public:
    explicit ThreadPool(size_t workers = std::thread::hardware_concurrency())
    {
        if (workers == 0)
            workers = 2;
        for (size_t i = 0; i < workers; ++i) {
            m_workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> job;
                    {
                        std::unique_lock lock(m_mutex);
                        m_cv.wait(lock, [this] { return m_stop || !m_jobs.empty(); });
                        if (m_stop && m_jobs.empty())
                            return;
                        job = std::move(m_jobs.front());
                        m_jobs.pop();
                    }
                    job();
                }
            });
        }
    }

    ~ThreadPool()
    {
        {
            std::lock_guard lock(m_mutex);
            m_stop = true;
        }
        m_cv.notify_all();
        for (auto& worker : m_workers)
            worker.join();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    template <typename Fn, typename... Args>
    auto enqueue(Fn&& fn, Args&&... args)
    {
        using Result = std::invoke_result_t<Fn, Args...>;
        auto task = std::make_shared<std::packaged_task<Result()>>(
            std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
        auto future = task->get_future();
        {
            std::lock_guard lock(m_mutex);
            m_jobs.emplace([task] { (*task)(); });
        }
        m_cv.notify_one();
        return future;
    }

private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_jobs;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_stop = false;
};

} // namespace PhotoStudio::Performance
