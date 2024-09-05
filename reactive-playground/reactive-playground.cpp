// reactive-playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <functional>
#include <coroutine>
#include <rpp/rpp.hpp>

template <typename T>
struct couroutine_generator
{
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;
    struct promise_type
    {
        T current_value;

        auto get_return_object() { return couroutine_generator{ coro_handle::from_promise(*this) }; }
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
    couroutine_generator(couroutine_generator const&) = delete;
    couroutine_generator(couroutine_generator&& rhs) : coro(rhs.coro) { rhs.coro = nullptr; }
    ~couroutine_generator()
    {
        if (coro)
            coro.destroy();
    }

private:
    couroutine_generator(coro_handle h) : coro(h) {}
    coro_handle coro;
};

couroutine_generator<int> fibonacci()
{
    int first = 0;
    int second = 1;

    while (true)
    {
        int temp = first + second;
        co_yield temp;
        first = second;
        second = temp;
    }
}


int main()
{
    namespace src = rpp::source;
    namespace ops = rpp::operators;

    auto observable$ = src::create<int>([](auto subscriber) {
            auto coro = fibonacci();

            while (!subscriber.is_disposed() && coro.move_next())
                subscriber.on_next(coro.current_value());
        })
        | ops::filter([](auto value) { return value % 2 == 0; })
        | ops::take_while([](auto value) { return value <= 1500; })
        | ops::map([](auto value) {
                return std::string{ "DINOSAUR no. " } + std::to_string(value);
            });

    observable$.subscribe(
        [](const std::string& value)
        {
            std::cout << value << std::endl;
        },
        []()
        {
            std::cout << "I Am Complete!" << std::endl;
        });

    return 0;
}
