#include "thread_pool.hpp"
#include <mutex>


thread_pool::thread_pool(const std::uint16_t& threads_count) :
    threads_(threads_count)
{

}


void thread_pool::run()
{
    if (run_ == false)
    {
        std::unique_lock<std::shared_mutex> lock(lock_);
        run_ = true;
        for (auto& ti : threads_) {
            ti.thread_ = std::thread(&thread_pool::worker, this, std::ref(ti));
        }
    }
}


void thread_pool::stop()
{
    if (run_)
    {
        std::unique_lock<std::shared_mutex> lock(lock_);
        tasks_.clear();
        run_ = false;
        for (auto& ti : threads_) {
            ti.thread_.join();
        }
    }
}


void thread_pool::wait()
{
    if (run_) {
        while (is_busy_thread());
    }
} 


void thread_pool::add_task(const std::function<void()>& task)
{
    if (run_)
    {
        std::unique_lock<std::shared_mutex> lock(lock_);
        tasks_.push_back(task);
    }
}


void thread_pool::add_task(std::function<void()>&& task)
{
    if (run_)
    {
        std::unique_lock<std::shared_mutex> lock(lock_);
        tasks_.push_back(std::move(task));
    }
}


void thread_pool::reset()
{
    wait();
    stop();
    threads_.clear();
    run_ = false;
    tasks_.clear();
}


std::uint16_t thread_pool::threads_count() const
{
    return threads_.size();
}


void thread_pool::threads_count(const std::uint16_t& threads_count)
{
    if (run_ == false) {
        threads_.resize(threads_count);
    }
}


bool thread_pool::is_free_thread() const
{
    if (run_)
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        for (const auto& t : threads_) {
            if (t.is_task_ == false) { return true; }
        }
    }

    return false;
}


bool thread_pool::is_busy_thread() const
{
    if (run_)
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        for (const auto& t : threads_) {
            if (t.is_task_ == true) { return true; }
        }
    }

    return false;
}


std::uint16_t thread_pool::free_threads_count() const
{
    std::uint16_t count = 0;

    if (run_)
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        for (const auto& t : threads_) {
            if (t.is_task_ == false) { count += 1; }
        }
    }

    return count;
}


std::uint16_t thread_pool::busy_threads_count() const
{
    std::uint16_t count = 0;
    if (run_)
    {
        std::shared_lock<std::shared_mutex> lock(lock_);
        for (const auto& t : threads_) {
            if (t.is_task_ == true) { count += 1; }
        }
    }

    return count;
}


std::size_t thread_pool::tasks_queue_size() const
{
    std::shared_lock<std::shared_mutex> lock(lock_);
    return tasks_.size();
}


bool thread_pool::is_running() const
{
    return run_;
}


void thread_pool::worker(thread_info& ti)
{
    std::function<void()> task;
    while (run_)
    {
        if (!tasks_.empty())
        {
            if (lock_.try_lock())
            {
                task = std::move(tasks_.front());
                tasks_.pop_front();
                ti.is_task_ = true;
                lock_.unlock();

                if (task) { task(); }

                lock_.lock();
                ti.is_task_ = false;
                lock_.unlock();
            }
        }
    }
}