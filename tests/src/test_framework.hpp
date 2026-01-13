#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace test {

struct TestCase {
    std::string name;
    std::function<bool()> func;
};

inline std::vector<TestCase>& get_tests() {
    static std::vector<TestCase> tests;
    return tests;
}

inline int register_test(const std::string& name, std::function<bool()> func) {
    get_tests().push_back({name, std::move(func)});
    return 0;
}

inline int run_all() {
    int passed = 0;
    int failed = 0;
    
    for (const auto& test : get_tests()) {
        std::cout << "Running: " << test.name << "... ";
        try {
            if (test.func()) {
                std::cout << "PASSED\n";
                ++passed;
            } else {
                std::cout << "FAILED\n";
                ++failed;
            }
        } catch (const std::exception& e) {
            std::cout << "EXCEPTION: " << e.what() << "\n";
            ++failed;
        }
    }
    
    std::cout << "\n=== Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed > 0 ? 1 : 0;
}

#define TEST(name) \
    static bool test_##name(); \
    static int _reg_##name = test::register_test(#name, test_##name); \
    static bool test_##name()

#define ASSERT(cond) do { if (!(cond)) { std::cerr << "  Assert failed: " #cond << "\n"; return false; } } while(0)
#define ASSERT_EQ(a, b) do { if ((a) != (b)) { std::cerr << "  Assert failed: " #a " == " #b << " (" << (a) << " != " << (b) << ")\n"; return false; } } while(0)

} // namespace test
