#include "../cpp/return_policy.h"
#include <cassert>
#include <cstdio>
int main() {
    using pikmin::return_task_ready;
    constexpr long long now=10000;
    assert(return_task_ready(1,1,true,0,now)); // walking / mushroom carry
    assert(return_task_ready(1,1,true,now,now));
    assert(!return_task_ready(1,1,true,now+1,now));
    assert(!return_task_ready(1,1,true,-1,now));
    assert(!return_task_ready(1,0,true,0,now));
    assert(!return_task_ready(1,2,true,0,now));
    assert(!return_task_ready(1,99,true,0,now));
    assert(!return_task_ready(1,1,false,0,now)); // no broad postcard/seed expansion
    assert(!return_task_ready(6,1,true,0,now)); // unstarted expedition
    assert(!return_task_ready(9,1,true,0,now)); // active mushroom
    assert(!return_task_ready(9,2,true,now+1,now));
    assert(!return_task_ready(8,1,true,0,now));
    assert(!return_task_ready(0,1,true,0,now));
    assert(!return_task_ready(1,1,true,0,0));
    assert(return_task_ready(6,1,false,now-1,now)); // legacy expedition return
    assert(return_task_ready(9,2,false,now-1,now)); // legacy due completion
    assert(pikmin::return_confirmed(true,false));
    assert(!pikmin::return_confirmed(true,true));
    assert(!pikmin::return_confirmed(false,false));
    assert(!pikmin::return_confirmed(false,true));
    std::puts("return policy: 20 checks passed");
}
