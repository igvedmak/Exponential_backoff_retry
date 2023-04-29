#include <future>
#include <cmath>
#include <atomic>
#include <thread>
#include <functional>
#include <chrono>
#include <cassert>

using namespace std::chrono;

namespace expbr {
    class Exponential_backoff_retry {
    public:

        /**
        * @brief                                                   C-Tor.
        * 
        * @param max_retries                                       Maximum number of times the function should be retried before giving up.
        * @param base_delay                                        The initial delay before retrying (in \b milliseconds).
        * @param backoff_factor                                    The multiplier applied to the base delay on each retry.
        */
        Exponential_backoff_retry(  const unsigned int max_retries, 
                                    milliseconds const& base_delay, 
                                    const double backoff_factor):   _maxRetries(max_retries), 
                                                                    _baseDelay(std::move(base_delay)), 
                                                                    _backoffFactor(backoff_factor) 
        {}

        /**
        * @brief                                                   This function calls until the desired result is achieved 
        *                                                          or the maximum number of retries is reached.
        * 
        * @tparam T                                                The expected return type of the function being called.
        * @tparam Func                                             The function signature being called.
        * @tparam Args                                             The argument types of the function being called.
        * @param t                                                 The expected return value.
        * @param func                                              The function to be called.
        * @param args                                              The arguments to be passed to the function being called.
        * @return auto = std::future<
        *          decltype(func(std::forward<Args>(args)...))>    A future object holding the result of the function call.
        */
        template<typename T, typename Func, typename... Args>
        auto operator()(T const& t, 
                        Func&& func, 
                        Args&&... args) {
            std::promise<decltype(func(std::forward<Args>(args)...))> promiseRes;  // Create a promise for the result
            auto futureRes = promiseRes.get_future(); // Create a future for the promise
        
            /* Start the function call asynchronously and return the future */
            std::thread work_thread([this, 
                                            t, 
                                            func,
                                            promiseRes = std::move(promiseRes), 
                                            args...]() mutable {
                std::atomic<unsigned int> retryCount = 0;
                decltype(func(std::forward<Args>(args)...)) result;
        
                do {
                    result = func(std::forward<Args>(args)...); // Call the function with the given arguments
                    if (result != t) {
                        /* If the result is not the desired result, wait for the specified delay */
                        std::this_thread::sleep_for(getDelay(retryCount));
                        ++retryCount;
                    } else {
                        /* If the result is the desired result, set the promise value and return */
                        promiseRes.set_value(result);
                        return;
                    }
                } while (retryCount < this->_maxRetries);
        
                /*  If the desired result was not obtained within the specified number of retries, 
                    set the promise value to the last result */
                promiseRes.set_value(result);
            });
            work_thread.join();  // wait for thread completion
            
            return futureRes;
        }

    private:
        
        /**
        * @brief                                                   Maximum number of times the function should be retried before .
        * 
        */
        unsigned int _maxRetries;
        
        /**
        * @brief                                                   The initial delay before retrying (in \b milliseconds).
        * 
        */
        milliseconds _baseDelay;
        
        /**
        * @brief                                                   The multiplier applied to the base delay on each retry.
        * 
        */
        double _backoffFactor;

        /**
        * @param args                                              This function calculates the delay for the next retry 
        *                                                          using an exponential backoff algorithm.                                
        * 
        * @param retryCount                                        The number of times the function has been retried so far.
        * @return milliseconds                                     The delay before the next retry (in \c milliseconds).
        */
        inline milliseconds getDelay(const int retryCount) const {
            return milliseconds(static_cast<long long>(this->_baseDelay.count() * std::pow(this->_backoffFactor, retryCount)));
        }
    };
}
