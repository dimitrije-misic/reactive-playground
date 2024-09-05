// reactive-playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <functional>
#include <rpp/rpp.hpp>
#include "coroutine_generator.h"

coroutine_generator<int> fibonacci()
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
