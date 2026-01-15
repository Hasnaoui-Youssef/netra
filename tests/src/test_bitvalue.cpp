#include "test_framework.hpp"
#include <components/components.hpp>
#include <boost/dynamic_bitset.hpp>

using namespace netra;

namespace {

bool bits_equal(const BitValue& a, const BitValue& b) {
    if (a.width() != b.width()) return false;
    for (std::uint32_t i = 0; i < a.width(); ++i) {
        if (a.get_bit(i) != b.get_bit(i)) return false;
    }
    return true;
}

} // namespace

TEST(bitvalue_index_access_and_update) {
    BitValue val(4);

    val[0] = true;
    val[3] = true;

    ASSERT(val[0]);
    ASSERT(val.get_bit(3));
    ASSERT(!val[1]);

    val.set_bit(1, true);
    ASSERT(val[1]);

    return true;
}

TEST(bitvalue_resize_and_clear) {
    BitValue val(2);
    val[0] = true;
    val[1] = false;

    val.resize(4);
    ASSERT_EQ(val.width(), 4u);
    ASSERT(val[0]);
    ASSERT(!val[1]);
    ASSERT(!val[2]);
    ASSERT(!val[3]);

    val.clear();
    for (std::uint32_t i = 0; i < val.width(); ++i) {
        ASSERT(!val[i]);
    }

    return true;
}

TEST(bitvalue_set_bits_with_dynamic_bitset) {
    BitValue val(6);

    boost::dynamic_bitset<> seq(3);
    seq.set(0);
    seq.set(2);
    val.set_bits(1, seq); // write positions 1,2,3

    ASSERT(!val[0]);
    ASSERT(val[1]);
    ASSERT(!val[2]); // seq[1] default false
    ASSERT(val[3]);

    boost::dynamic_bitset<> reverse(2);
    reverse.set(0);
    val.set_bits(4, 3, reverse); // descending write to 4 then 3

    ASSERT(val[4]);
    ASSERT(!val[3]); // overwritten

    return true;
}

TEST(bitvalue_copy_and_range) {
    BitValue original(5);
    original.set_bits(0, boost::dynamic_bitset<>(5, 0b10101));

    BitValue copy = original;
    ASSERT(bits_equal(original, copy));

    BitValue forward = original.range(1, 3); // expected bits 1..3 => 010
    ASSERT_EQ(forward.width(), 3u);
    ASSERT(!forward[0]);
    ASSERT(forward[1]);
    ASSERT(!forward[2]);

    BitValue reverse = original.range(4, 2); // reverse order: bits 4,3,2 => 101 -> indices 0..2
    ASSERT_EQ(reverse.width(), 3u);
    ASSERT(reverse[0]);
    ASSERT(!reverse[1]);
    ASSERT(reverse[2]);

    return true;
}
