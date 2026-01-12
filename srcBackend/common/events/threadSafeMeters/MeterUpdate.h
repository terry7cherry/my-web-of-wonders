#pragma once
#include <functional>
#include <vector>

class MeterUpdate
{
public:
    using Callback = std::function<void()>;

    static void subscribe(Callback cb)
    {
        subscribers_.push_back(std::move(cb));
    }

    static void triggerUpdate()
    {
        for (auto& cb : subscribers_)
        {
            cb();
        }
    }

private:
    inline static thread_local std::vector<Callback> subscribers_;
};
