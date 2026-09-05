#include "../cpp/dispatch_safety.h"
#include <cassert>
#include <cstdio>
using namespace pikmin;
int main() {
    for (int64_t ms : {-2000, -1, 0, 2001, 300000}) assert(!batch_duration_safe(ms));
    assert(batch_duration_safe(1) && batch_duration_safe(2000));
    assert(dispatch_status_available(1) && dispatch_status_available(32));
    assert(!dispatch_status_available(2) && !dispatch_status_available(0) && !dispatch_status_available(999));
    assert(gift_candidate_allowed("owner", "owner", true));
    assert(!gift_candidate_allowed("owner", "other", true));
    assert(!gift_candidate_allowed("", "", true));
    assert(!gift_candidate_allowed("owner", "owner", false));
    assert(!(dispatch_status_available(2) && gift_candidate_allowed("owner", "owner", true)));
    assert(dispatch_ids("a,b").size() == 2);
    for (auto value : {"", "a,", ",a", "a,a", "a,,b", "<empty>", "a\tb"}) assert(dispatch_ids(value).empty());
    assert(same_dispatch_team({"a", "b"}, {"b", "a"}));
    assert(!same_dispatch_team({"a"}, {"b"}) && !same_dispatch_team({}, {}));
    DispatchReservations r;
    assert(!r.reserve("", {"a"}) && !r.reserve("one", {}) && !r.reserve("one", {"a", "a"}));
    assert(r.reserve("one", {"a", "b"}));
    assert(!r.reserve("two", {"b", "c"}) && r.size() == 2); // all-or-nothing
    assert(r.permits("a", "one") && !r.permits("a", "two"));
    r.sent("one", 100);
    r.cancel("one"); // a stop/fault must not free potentially in-flight IDs
    assert(!r.reserve("one", {"new-id"})); // no duplicate expedition with a different team
    assert(!r.permits("a", "one") && !r.permits("a", "two"));
    r.observe("a", 1); // stale available snapshot cannot free a sent team
    r.observe("a", 999);
    r.observe("a", 1);
    assert(r.size() == 2);
    r.observe("a", 2); r.observe("a", 32);
    assert(r.permits("a", "two") && r.size() == 1);
    r.reconcile_tasks({}, 100); assert(r.size() == 1); // pre-send projection
    r.reconcile_tasks({"one"}, 1000); assert(r.size() == 1); // no arbitrary TTL
    r.reconcile_tasks({}, 1001); assert(r.size() == 0); // fast collected, busy tick missed
    assert(r.reserve("batch", {"a"}));
    r.cancel("batch"); assert(r.size() == 0);
    assert(r.reserve("one", {"a"}) && r.reserve("two", {"b"}) && r.reserve("three", {"c"}));
    r.sent("one", 2000); r.sent("two", 2000); r.sent("three", 2000);
    assert(r.size() == 3 && !r.reserve("four", {"a", "d"}));
    std::puts("PASS: duration boundaries; strict IDs; exact teams; concurrent reservations; safe release");
}
