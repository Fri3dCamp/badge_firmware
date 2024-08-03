#pragma once

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

// clang-format off
#define EVENT_CREATE_START(name)    \
struct name                         \
{

#define EVENT_CREATE_TYPES_START()  \
    enum EventType                  \
    {                               \
        Shutdown,

#define EVENT_CREATE_TYPES_END()    \
    };                              \
                                    \
    EventType eventType;

#define EVENT_CREATE_END()          \
};
// clang-format on

namespace Fri3d::Application
{

template <typename T> class CThread
{
private:
    // Use lower case to not interfere with static TAG definitions
    const char *tag;

    typedef std::queue<T> CEvents;
    CEvents events;
    std::mutex eventsMutex;
    std::atomic<bool> newEvents;

    std::thread worker;
    std::mutex workerMutex;

    void work()
    {
        bool running = true;
        CEvents processing;

        while (running)
        {
            ESP_LOGV(this->tag, "Waiting on new events");
            this->newEvents.wait(false);
            {
                ESP_LOGV(this->tag, "New events received");
                std::lock_guard lock(this->eventsMutex);

                std::swap(processing, this->events);
                this->newEvents = false;
            }

            while (!processing.empty())
            {
                auto event = processing.front();
                processing.pop();

                this->onEvent(event);

                if (event.eventType == T::Shutdown)
                {
                    running = false;
                    // Stop processing immediately
                    break;
                }
            }
        }
    }

protected:
    void sendEvent(const T &event)
    {
        {
            std::lock_guard lock(this->eventsMutex);
            this->events.push(event);
        }

        this->newEvents = true;
        this->newEvents.notify_all();
    }

    // Event handler to be declared by child classes, this is guaranteed to be called only one at a time
    virtual void onEvent(const T &event) = 0;

public:
    explicit CThread(const char *tag)
        : tag(tag)
        , newEvents(false)
    {
    }

    void start()
    {
        std::lock_guard lock(this->workerMutex);
        if (this->worker.joinable())
        {
            throw std::runtime_error("Already running");
        }

        // Make sure the event queue is empty before we start
        this->events = CEvents();

        this->worker = std::thread(&CThread::work, this);
    }

    void stop()
    {
        std::lock_guard lock(this->workerMutex);
        if (this->worker.joinable())
        {
            auto event = T();
            event.eventType = T::Shutdown;
            this->sendEvent(event);

            // Wait for the thread to stop
            this->worker.join();

            // Clear the queue
            this->events = CEvents();
        }
    }
};

} // namespace Fri3d::Application
