#pragma once
#include <coroutine>

template <typename T>
struct coroutine_generator
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;
    struct promise_type
    {
        T current_value;

        auto get_return_object() { return coroutine_generator{ coro_handle::from_promise(*this) }; }
        auto initial_suspend() { return std::suspend_always{}; }
        auto final_suspend() noexcept { return std::suspend_always{}; }
        void unhandled_exception() { std::terminate(); }
        auto return_void() { return std::suspend_never{}; }

        auto yield_value(T value)
        {
            current_value = value;
            return std::suspend_always{};
        }
    };
    bool move_next() { return coro ? (coro.resume(), !coro.done()) : false; }
    T current_value() { return coro.promise().current_value; }
    coroutine_generator(coroutine_generator const&) = delete;
    coroutine_generator(coroutine_generator&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~coroutine_generator()
    {
        if (coro)
            coro.destroy();
    }

private:
    coroutine_generator(coro_handle h) : coro(h) {}
    coro_handle coro;
};