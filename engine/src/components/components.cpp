#include "components/components.hpp"

namespace netra {

BitValue::BitValue() : m_width(0) {}

BitValue::BitValue(std::uint32_t width) : m_width(width) {
    m_bits.resize((width + 7) / 8, 0);
}

void BitValue::set_bit(std::uint32_t idx, bool val) {
    if (idx >= m_width) return;
    if (val) {
        m_bits[idx / 8] |= (1 << (idx % 8));
    } else {
        m_bits[idx / 8] &= ~(1 << (idx % 8));
    }
}

bool BitValue::get_bit(std::uint32_t idx) const {
    if (idx >= m_width) return false;
    return (m_bits[idx / 8] >> (idx % 8)) & 1;
}

std::uint32_t BitValue::width() const {
    return m_width;
}

void BitValue::resize(std::uint32_t new_width) {
    m_width = new_width;
    m_bits.resize((new_width + 7) / 8, 0);
}

void BitValue::clear() {
    std::fill(m_bits.begin(), m_bits.end(), 0);
}

} // namespace netra
