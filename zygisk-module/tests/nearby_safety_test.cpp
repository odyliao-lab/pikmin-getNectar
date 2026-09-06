#include "../cpp/nearby_safety.h"
#include <cassert>
#include <cstdio>
#include <limits>
#include <string>
#include <initializer_list>
using namespace pikmin;
int main() {
    for (int64_t ms : {-2000, -1, 0, 300001, 519535566, 519560925, 519568240}) assert(!nearby_duration_safe(ms));
    for (int64_t ms : {1, 2000, 120000, 120001, 180000, 299999, 300000}) assert(nearby_duration_safe(ms));
    NearbyLocationGate gate;
    NearbyFix taiwan{true,25,121,25,121,10000};
    gate.observe(taiwan,10000,10000); assert(!gate.ready(10000));
    gate.observe(taiwan,11000,11000); gate.observe(taiwan,14000,14000);
    assert(!gate.ready(14000)); // repeated cached fix cannot open gate
    taiwan.raw_wall_ms=15000; gate.observe(taiwan,15000,15000);
    taiwan.raw_wall_ms=16000; gate.observe(taiwan,16000,16000); assert(gate.ready(16000));
    const auto before_jump=gate.epoch();
    NearbyFix japan{true,35,139,28,127,17000};
    gate.observe(japan,17000,17000); assert(!gate.ready(17000) && gate.epoch()!=before_jump);
    for (double intermediate : {30.,32.,34.}) {
        japan.game_lat=intermediate; japan.raw_wall_ms+=1000;
        gate.observe(japan,japan.raw_wall_ms,japan.raw_wall_ms); assert(!gate.ready(japan.raw_wall_ms));
    }
    japan.game_lat=35; japan.game_lng=139;
    for (int i=0;i<4;++i) { japan.raw_wall_ms+=1000; gate.observe(japan,japan.raw_wall_ms,japan.raw_wall_ms); }
    assert(gate.ready(japan.raw_wall_ms)); // catches up, 3 distinct fixes + >=3 seconds
    assert(!gate.ready(japan.raw_wall_ms+2001)); // stopped heartbeat
    auto epoch=gate.epoch();
    japan.raw_lat=japan.game_lat=36; japan.raw_wall_ms+=1000;
    gate.observe(japan,japan.raw_wall_ms,japan.raw_wall_ms);
    assert(!gate.ready(japan.raw_wall_ms) && gate.epoch()!=epoch); // instant full jump still settles
    gate.observe(japan,japan.raw_wall_ms+6000,japan.raw_wall_ms+6000); assert(!gate.ready(japan.raw_wall_ms+6000));
    assert(std::string(gate.reason())=="stale-game-location");
    japan.raw_wall_ms=999999; gate.observe(japan,40000,40000); assert(!gate.ready(40000));
    japan.valid=false; gate.observe(japan,41000,41000); assert(!gate.ready(41000));
    japan.valid=true; japan.raw_lat=std::numeric_limits<double>::quiet_NaN();
    gate.observe(japan,42000,42000); assert(!gate.ready(42000));
    NearbyFix origin{true,0,0,0,0,1000}; NearbyLocationGate zero;
    for (int i=0;i<4;++i) { origin.raw_wall_ms+=1000; zero.observe(origin,origin.raw_wall_ms,origin.raw_wall_ms); }
    assert(zero.ready(5000)); // equator/prime meridian are valid coordinates
    epoch=zero.epoch(); zero.observe(origin,4000,6000); assert(!zero.ready(6000) && epoch!=zero.epoch());
    // Real v152 sample: Unix=1788695172167, boot time ~757000, raw=755828.
    // Monotonic can differ from boot time after suspend; never use either
    // Unix or monotonic for raw-fix age, nor calibrate stale input to 'now'.
    NearbyLocationGate boot;
    NearbyFix actual{true,24.1663475,120.6338120,24.1663475,120.6338120,755828};
    for (int i=0; i<4; ++i) {
        actual.raw_wall_ms=755828+i*1000;
        boot.observe(actual,757000+i*1000,700000+i*1000);
    }
    assert(boot.ready(703000));
    boot.observe(actual,821000,703001); // one minute suspended, stale raw fix
    assert(!boot.ready(703001));
    boot.observe(actual,-1,703002); assert(!boot.ready(703002));
    boot.observe(actual,1788695172167LL,703003); assert(!boot.ready(703003));
    boot.observe(actual,700000,703004); assert(!boot.ready(703004)); // wrong clock/future
    puts("PASS nearby safety: 300s inclusive boundary, 300001ms rejected, 144h failures, staged teleport, stale/future fixes, real boot-time sample and suspend");
}
