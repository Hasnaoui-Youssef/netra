#pragma once

#include <cstdint>
#include <limits>

namespace netra {

using EntityID = std::uint32_t;
using ComponentTypeID = std::uint32_t;

constexpr EntityID NullEntity = std::numeric_limits<EntityID>::max();

enum class PortDirection : std::uint8_t {
    In,
    Out,
    InOut
};

// Compile-time type ID generation
namespace detail {
    inline ComponentTypeID next_component_type_id() {
        static ComponentTypeID id = 0;
        return id++;
    }
}

template<typename T>
inline ComponentTypeID get_component_type_id() {
    static ComponentTypeID id = detail::next_component_type_id();
    return id;
}

} // namespace netra
