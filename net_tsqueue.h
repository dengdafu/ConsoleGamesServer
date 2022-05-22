#ifndef NET_TS_QUEUE_
#define NET_TS_QUEUE_

#include <condition_variable>
#include <mutex>
#include "net_common.h"

namespace net {
    template<typename T>
    class TsQueue {
        public:
        TsQueue() = default;
        TsQueue(const TsQueue<T>&) = delete;
        virtual ~TsQueue() { Clear(); }

        void Clear() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            deq_queue_.clear();
        };

        const T& Front() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            return deq_queue_.front();
        }

        const T& Back() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            return deq_queue_.back();
        }

        T PopFront() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            auto t = std::move(deq_queue_.front());
            deq_queue_.pop_front();
            return t;
        }

        T PopBack() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            auto t = std::move(deq_queue_.back());
            deq_queue_.pop_back();
            return t;
        }

        void PushBack(const T& item) {
            std::lock_guard<std::mutex> lock(mux_queue_);
            deq_queue_.emplace_back(std::move(item));
        }

        void PushFront(const T& item) {
            std::lock_guard<std::mutex> lock(mux_queue_);
            deq_queue_.emplace_front(std::move(item));
        }

        bool Empty() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            return deq_queue_.empty();
        }

        size_t Count() {
            std::lock_guard<std::mutex> lock(mux_queue_);
            return deq_queue_.size();
        }

        protected:
        std::mutex mux_queue_;
        std::deque<T> deq_queue_;
    };
}

#endif //NET_TS_QUEUE_