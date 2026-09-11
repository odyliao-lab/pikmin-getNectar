#include "../cpp/route_policy.h"
#include <cassert>
#include <iostream>
int main(){
    const std::string token="12345678-1234-1234-1234-123456789abc";
    const std::string account(64,'a');
    assert(pikmin::RouteLease::parse("v2 9 20000 "+token+" walk "+account+"\n",9,10000).account==account);
    assert(!pikmin::RouteLease::parse("v1 9 20000 "+token+" walk\n",9,10000).valid());
    assert(!pikmin::RouteLease::parse("v2 9 20000 "+token+" walk bad\n",9,10000).valid());
    for(auto prefix:{"v2 8 20000 ","v2 9 9999 ","v2 9 30000 "})assert(!pikmin::RouteLease::parse(std::string(prefix)+token+" walk "+account,9,10000).valid());
    for(auto raw:{"v1 9 20000 "+token+" walk extra", "v1 8 20000 "+token+" walk", "v1 9 9999 "+token+" walk", "v1 9 30000 "+token+" walk", "v1 9 20000 "+token+" armed"})
        assert(!pikmin::RouteLease::parse(raw,9,10000).valid());
    pikmin::RouteLocationGate g;
    for(int i=0;i<25;i++) {double raw=i*0.000045,game=std::max(0,i-5)*0.000045;g.observe({true,0,raw,0,game,1000+i*1000},1000+i*1000);}
    assert(g.ready(25000)); auto epoch=g.epoch();
    g.observe({true,20,120,0,0.001,26000},26000);assert(!g.ready(26000));assert(g.epoch()!=epoch);
    g.reset();for(int i=0;i<20;i++)g.observe({true,0,0,0,0,1000+i*1000},1000+i*1000);
    assert(g.ready(20000));assert(!g.ready(22001));
    g.observe({true,0,0,0,0,19000},21000);assert(!g.ready(21000));
    g.reset();g.observe({true,0,0,0,0,1000},1000);for(int i=1;i<20;i++)g.observe({true,0,0,0,0,1000},1000+i*100);
    assert(!g.ready(2900));
    std::cout<<"PASS route lease, continuous delayed game trajectory, jumps, stale and duplicate fixes\n";
}
