#pragma once

#include <concepts>
namespace netra {
template <typename Pred, typename... Args>
concept Predicate = std::invocable<Pred, Args...>
    && std::same_as<std::invoke_result_t<Pred, Args...>, bool>;
}
