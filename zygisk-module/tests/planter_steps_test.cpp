#include "../cpp/planter_policy.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace pikmin::planter;
    assert(parse_steps("1000 3000 5000\n") == default_steps);
    assert(!step_allowed(default_steps, 100)); assert(!step_allowed(default_steps, 10000));
    assert(step_allowed(default_steps, 1000)); assert(step_allowed(default_steps, 3000));
    assert(step_allowed(default_steps, 5000)); assert(!step_allowed(default_steps, 2500));
    assert(parse_steps("none") == 0); assert(parse_steps("") == 0);
    assert(parse_steps("1000 invalid") == 0); assert(parse_steps("none 1000") == 0);
    assert(parse_steps("1000 1000") == 2); assert(parse_steps("100\t10000 other") == 49);
    assert(step_allowed(32, 2500)); assert(!step_allowed(32, 1000));
    assert(!step_allowed(63, 0)); assert(!step_allowed(63, -1));
    assert(parse_steps(std::string(129, ' ')) == 0);
    assert(mature(10, 100, 100, true)); // Excluding 100 from planting must not affect plucking.
    assert(!step_allowed(0, 1000));
    puts("PASS: 20 planter step filter cases");
}
