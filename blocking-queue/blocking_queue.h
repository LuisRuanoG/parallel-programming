#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockingQueue {
private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
    bool finished = false;

public:

    void push(T item) {
        std::unique_lock<std::mutex> lock(m);
        q.push(item);
        cv.notify_one();
    }

    bool pop(T &item) {
        std::unique_lock<std::mutex> lock(m);

        while(q.empty() && !finished)
            cv.wait(lock);

        if(q.empty())
            return false;

        item = q.front();
        q.pop();
        return true;
    }

    void set_finished() {
        std::unique_lock<std::mutex> lock(m);
        finished = true;
        cv.notify_all();
    }
};

#endif