#pragma once

#include <functional>

class IMainThreadDispatcher
{
public:
    virtual ~IMainThreadDispatcher() = default;

    virtual void dispatch(std::function<void()> task) = 0;
};
