#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__
#include <cinttypes>
#include <thread>
#include <vector>
#include <list>
#include <shared_mutex>
#include <functional>


class thread_pool
{
    public:
        thread_pool() = default;
        thread_pool(const std::uint16_t& threads_count);
        thread_pool(const thread_pool& obj) = delete;
        thread_pool(thread_pool&& obj) = delete;
        ~thread_pool() = default;

        thread_pool& operator=(const thread_pool& obj) = delete;
        thread_pool& operator=(thread_pool&& obj) = delete;

        void reset();
        void run();
        void stop();
        void wait();
        void add_task(const std::function<void()>& task);
        void add_task(std::function<void()>&& task);
        std::uint16_t threads_count() const;
        void threads_count(const std::uint16_t& threads_count);
        bool is_free_thread() const;
        bool is_busy_thread() const;
        std::uint16_t free_threads_count() const;
        std::uint16_t busy_threads_count() const;
        std::size_t tasks_queue_size() const;
        bool is_running() const;


    private:
        struct thread_info
        {
            std::thread thread_;
            bool is_task_ = false;
        };

        void worker(thread_info& ti);

        bool run_ = false;
        std::vector<thread_info> threads_;
        std::list<std::function<void()>> tasks_;
        mutable std::shared_mutex lock_;
};



#endif