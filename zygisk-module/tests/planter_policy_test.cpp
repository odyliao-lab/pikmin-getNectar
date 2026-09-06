#include "../cpp/planter_policy.h"
#include <cassert>
#include <cstdio>
int main() {
    using namespace pikmin::planter;
    assert(permanent_slot(0, 0, 2)); assert(permanent_slot(0, 1, 2));
    assert(!permanent_slot(1, 2, 4)); assert(!permanent_slot(4, 2, 4));
    assert(!permanent_slot(0, -1, 2)); assert(!permanent_slot(0, 2, 2));
    assert(!permanent_slot(0, 0, 99));
    assert(mature(100, 1000, 1000, true));
    assert(!mature(100, 999, 1000, true)); assert(!mature(0, 1000, 1000, true));
    assert(!mature(100, 1000, 1000, false)); assert(!mature(100, 0, 0, true));
    assert(can_plant(0, false, false, 1000, false, true));
    assert(!can_plant(1, false, false, 1000, false, true));
    assert(!can_plant(0, true, false, 1000, false, true));
    assert(!can_plant(0, false, true, 1000, false, true));
    assert(!can_plant(0, false, false, 1000, true, true));
    assert(!can_plant(0, false, false, 1000, false, false));
    assert(!can_plant(0, false, false, 0, false, true));
    assert(storage_room(299, 300)); assert(!storage_room(300, 300));
    assert(!storage_room(301, 300)); assert(!storage_room(0, 0));
    assert(!storage_room(-1, 300));
    puts("PASS: 24 planter policy cases");
}
