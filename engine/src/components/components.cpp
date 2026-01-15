#include "components/components.hpp"
#include <algorithm>

namespace netra {

BitValue::BitValue() = default;

BitValue::BitValue(std::uint32_t width) : m_bits(width) {}

void BitValue::set_bit(std::uint32_t idx, bool val) {
    if (idx >= m_bits.size()) return;
    m_bits.set(idx, val);
}

void BitValue::set_bits(std::uint32_t start_idx, std::uint32_t end_idx, const boost::dynamic_bitset<>& val){
    if (val.empty()) return;
    auto min_idx = std::min(start_idx, end_idx);
    auto max_idx = std::max(start_idx, end_idx);
    if (max_idx >= m_bits.size()) return;
    if (val.size() != (max_idx - min_idx + 1)) return;

    if (start_idx <= end_idx) {
        for (std::size_t i = 0; i < val.size(); ++i) {
            set_bit(start_idx + static_cast<std::uint32_t>(i), val[i]);
        }
    } else {
        for (std::size_t i = 0; i < val.size(); ++i) {
            set_bit(start_idx - static_cast<std::uint32_t>(i), val[i]);
        }
    }
}

void BitValue::set_bits(std::uint32_t start_idx, const boost::dynamic_bitset<>& val) {
    if (val.empty()) return;
    if (start_idx >= m_bits.size()) return;
    if (start_idx + val.size() > m_bits.size()) return;

    for (std::size_t i = 0; i < val.size(); ++i) {
        set_bit(start_idx + static_cast<std::uint32_t>(i), val[i]);
    }
}

bool BitValue::get_bit(std::uint32_t idx) const {
    if (idx >= m_bits.size()) return false;
    return m_bits.test(idx);
}

std::uint32_t BitValue::width() const {
    return static_cast<std::uint32_t>(m_bits.size());
}

void BitValue::resize(std::uint32_t new_width) {
    m_bits.resize(new_width);
}

void BitValue::clear() {
    m_bits.reset();
}

BitValue BitValue::range(std::uint32_t start_idx, std::uint32_t end_idx) {
    if (m_bits.empty()) return BitValue();

    auto min_idx = std::min(start_idx, end_idx);
    auto max_idx = std::min<std::uint32_t>(std::max(start_idx, end_idx), width() ? width() - 1 : 0);
    if (min_idx >= m_bits.size()) return BitValue();

    std::uint32_t len = max_idx - min_idx + 1;
    BitValue result(len);

    if (start_idx <= end_idx) {
        for (std::uint32_t i = 0; i < len; ++i) {
            result.set_bit(i, get_bit(min_idx + i));
        }
    } else {
        for (std::uint32_t i = 0; i < len; ++i) {
            result.set_bit(i, get_bit(start_idx - i));
        }
    }

    return result;
}

BitValue BitValue::range(std::uint32_t start_idx) {
    if (m_bits.empty() || start_idx >= m_bits.size()) return BitValue();
    return range(start_idx, static_cast<std::uint32_t>(m_bits.size() - 1));
}

boost::dynamic_bitset<>::reference BitValue::operator[](std::uint32_t idx) {
    return m_bits[idx];
}

bool BitValue::operator[](std::uint32_t idx) const {
    return get_bit(idx);
}

} // namespace netra
