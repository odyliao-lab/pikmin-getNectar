#pragma once
#include "nearby_safety.h"
#include <cstdio>
#include <string>
#include <deque>

namespace pikmin {
struct RouteLease {
    int pid{}; long long until{}; std::string token, phase, account;
    static RouteLease parse(const std::string &raw, int expected, long long now) {
        RouteLease r; char token[65]{},phase[16]{},account[65]{}; int end{};
        if(raw.size()>200 || std::sscanf(raw.c_str(),"v2 %d %lld %64s %15s %64s %n",&r.pid,&r.until,token,phase,account,&end)!=5
                || end!=static_cast<int>(raw.size()) || r.pid!=expected || now<=0 || r.until<=now || r.until-now>15000) return {};
        r.token=token;r.phase=phase;r.account=account;
        if(r.account.size()!=64||r.account.find_first_not_of("0123456789abcdef")!=std::string::npos)return {};
        if(r.token.size()!=36 || r.token.find_first_not_of("0123456789abcdef-")!=std::string::npos
                || (r.phase!="prepare"&&r.phase!="walk"&&r.phase!="hold")) return {};
        return r;
    }
    bool valid() const { return !token.empty(); }
};
// Dedicated route gate: match processed game position to a recent, continuous
// raw trajectory. It does NOT loosen NearbyLocationGate's existing 8m rule.
class RouteLocationGate {
    struct Sample { NearbyFix f; int64_t elapsed; };
    std::deque<Sample> history;
    uint64_t epoch_{}; bool ready_{}; int64_t seen_{};
public:
    void reset() { history.clear();ready_=false;seen_=0;++epoch_; }
    void observe(const NearbyFix &f,int64_t elapsed) {
        if(!f.valid || !valid_point(f.raw_lat,f.raw_lng)||!valid_point(f.game_lat,f.game_lng)
                ||f.raw_wall_ms<=0||elapsed<f.raw_wall_ms||elapsed-f.raw_wall_ms>2000) {reset();return;}
        if(!history.empty()) {
            const auto &p=history.back().f; auto dt=f.raw_wall_ms-p.raw_wall_ms;
            if(elapsed<seen_||elapsed-seen_>2000||dt<0||dt>2000
                    ||nearby_metres(p.raw_lat,p.raw_lng,f.raw_lat,f.raw_lng)>7.0*dt/1000+2
                    ||nearby_metres(p.game_lat,p.game_lng,f.game_lat,f.game_lng)>7.0*(elapsed-history.back().elapsed)/1000+2) {
                reset(); return;
            }
            if(dt==0) { seen_=elapsed;return; }
        }
        seen_=elapsed;history.push_back({f,elapsed});
        while(history.size()>1 && elapsed-history.front().elapsed>12000)history.pop_front();
        bool matches=false;
        for(const auto &p:history) if(elapsed-p.elapsed<=10000
                &&nearby_metres(p.f.raw_lat,p.f.raw_lng,f.game_lat,f.game_lng)<=8) {matches=true;break;}
        ready_=history.size()>=8 && elapsed-history.front().elapsed>=10000 && matches
                &&nearby_metres(f.raw_lat,f.raw_lng,f.game_lat,f.game_lng)<=60;
        if(!matches && history.size()>1) {reset();}
    }
    bool ready(int64_t elapsed) const {return ready_ && elapsed>=seen_ && elapsed-seen_<=2000;}
    uint64_t epoch() const {return epoch_;}
};
}
