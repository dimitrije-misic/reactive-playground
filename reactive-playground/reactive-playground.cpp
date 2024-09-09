// reactive-playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <functional>
#include <concepts>
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

namespace rpp::source
{
    template <typename G, typename T>
    concept generator = requires (G g)
    {
        { std::invocable<G> };
        { g() } -> std::convertible_to<coroutine_generator<T>>;
    };

    template <typename T>
    auto from_coroutine(generator<T> auto generator)
    {
        return create<T>([generator](auto subscriber)
            {
                auto coroutine = generator();

                while (!subscriber.is_disposed() && coroutine.move_next())
                    subscriber.on_next(coroutine.current_value());
            });
    }
}

int main()
{
    namespace src = rpp::source;
    namespace ops = rpp::operators;

    auto observable$ = src::from_coroutine<int>(fibonacci)
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
