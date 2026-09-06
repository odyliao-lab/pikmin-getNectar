#include "../cpp/nearby_policy.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace pikmin;
    assert(nearby_selection("fruit seed") == 3);
    assert(nearby_selection("gift") == 4);
    assert(nearby_selection("gift fruit seed") == 7);
    for (const auto &bad : {"", "none", "fruit garbage", "all", "farm", "gift none"})
        assert(nearby_selection(bad) == 0);
    assert(nearby_selection(std::string(129, ' ')) == 0);
    for (unsigned mask = 0; mask < 8; ++mask) {
        assert(nearby_kind_allowed("all", mask, "fruit") == ((mask & 1) != 0));
        assert(nearby_kind_allowed("all", mask, "seed") == ((mask & 2) != 0));
        assert(nearby_kind_allowed("all", mask, "gift") == ((mask & 4) != 0));
        assert(!nearby_kind_allowed("all", mask, "unknown"));
        assert(nearby_kind_allowed("farm", mask, "fruit"));
        assert(nearby_kind_allowed("farm", mask, "seed"));
        assert(!nearby_kind_allowed("farm", mask, "gift"));
        assert(!nearby_kind_allowed("seed", mask, "fruit"));
        assert(nearby_kind_allowed("seed", mask, "seed") == ((mask & 2) != 0));
    }
    std::cout << "PASS: nearby parser, 8 masks, farm isolation, legacy restrictions\n";
}
