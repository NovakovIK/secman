#ifndef SECMAN_TREAD_POOL_HPP
#define SECMAN_TREAD_POOL_HPP


#include <functional>
#include <thread>
#include <atomic>
#include <vector>
#include <memory>
#include <exception>
#include <future>
#include <mutex>
#include <queue>



// thread pool to run user's functors with signature
//      ret func(int id, other_params)
// where id is the index of the thread that runs the functor
// ret is some return type


namespace tp
{
    namespace detail
    {
        template <typename T>
        class Queue
        {

        public:
            bool push(T const & value);
            bool pop(T & v);  // deletes the retrieved element, do not use for non integral types
            bool empty();

        private:
            std::queue<T> q;
            std::mutex mutex;
        };

        template<typename T>
        bool Queue<T>::push(const T &value)
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            this->q.push(value);
            return true;
        }

        template<typename T>
        bool Queue<T>::pop(T &v)
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            if (this->q.empty())
                return false;
            v = this->q.front();
            this->q.pop();
            return true;
        }

        template<typename T>
        bool Queue<T>::empty()
        {
            std::unique_lock<std::mutex> lock(this->mutex);
            return this->q.empty();
        }
    }

    class thread_pool
    {

    public:

        thread_pool();
        explicit thread_pool(int nThreads);
        ~thread_pool();       // the destructor waits for all the functions in the queue to be finished
        int size();           // get the number of running threads in the pool
        int n_idle();         // number of idle threads
        std::thread & get_thread(int i);

        // change the number of threads in the pool
        // should be called from one thread, otherwise be careful to not interleave, also with this->stop()
        // nThreads must be >= 0
        void resize(int nThreads);

        // empty the queue
        void clear_queue();


        // pops a functional wrapper to the original function
        std::function<void(int)> pop();


        // wait for all computing threads to finish and stop all threads
        // may be called asynchronously to not pause the calling thread while waiting
        // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
        void stop(bool isWait = false);

        template<typename F, typename... Rest>
        auto push(F && f, Rest&&... rest) -> std::future<decltype(f(0, rest...))>
        {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>
                    (
                            std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Rest>(rest)...)
                    );
            auto _f = new std::function<void(int id)>([pck](int id) { (*pck)(id); });
            this->q.push(_f);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }
        // run the user's function that excepts argument int - id of the running thread. returned value is templatized
        // operator returns std::future, where the user can get the result and rethrow the catched exceptins
        template<typename F>
        auto push(F && f) -> std::future<decltype(f(0))>
        {
            auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
            auto _f = new std::function<void(int id)>([pck](int id){ (*pck)(id); });
            this->q.push(_f);
            std::unique_lock<std::mutex> lock(this->mutex);
            this->cv.notify_one();
            return pck->get_future();
        }



    private:

        void set_thread(int i);

        std::vector<std::unique_ptr<std::thread>> threads;
        std::vector<std::shared_ptr<std::atomic<bool>>> flags;
        detail::Queue<std::function<void(int id)> *> q;
        std::atomic<bool> isDone;
        std::atomic<bool> isStop;
        std::atomic<int> nWaiting;  // how many threads are waiting

        std::mutex mutex;
        std::condition_variable cv;
    };

}



#endif