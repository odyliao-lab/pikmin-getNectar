#include "../cpp/planter_recovery.h"
#include <cassert>
#include <iostream>
using namespace pikmin::planter;
int main(){
    Ledger l;assert(parse_ledger("1000\tpluck\tseed-1\t0\n",l));
    assert(!parse_ledger("1000 pluck seed-1 0 extra",l));
    assert(!parse_ledger("0 pluck seed-1 0",l));assert(!parse_ledger("1 drop seed-1 0",l));
    assert(!parse_ledger("1 plant seed-1 8",l));assert(!parse_ledger("1 plant seed;evil 0",l));
    RecoveryRequest r;assert(parse_recovery("v1 request-1 42 2000 1000 pluck seed-1 0",r));
    assert(fresh_recovery(r,42,2000));assert(fresh_recovery(r,42,92000));
    assert(!fresh_recovery(r,43,3000));assert(!fresh_recovery(r,42,1999));assert(!fresh_recovery(r,42,92001));
    assert(!parse_recovery("v2 request-1 42 2000 1000 pluck seed-1 0",r));
    assert(parse_recovery("v1 request-1 42 2000 0 none none -1",r));
    assert(!parse_recovery("v1 request-1 42 2000 0 none none 0",r));
    assert(parse_ledger("1000 pluck seed-1 0",l));
    assert(recovery_proof(l,true,false,true,false,0,false,false)=="pluck-target-retired");
    assert(!can_recover(recovery_proof(l,true,false,true,true,100,true,true)));
    assert(!can_recover(recovery_proof(l,true,true,true,false,0,false,false)));
    assert(!can_recover(recovery_proof(l,false,false,true,false,0,false,false)));
    assert(!can_recover(recovery_proof(l,true,false,false,false,0,false,false)));
    assert(!can_recover(recovery_proof(l,true,false,true,false,0,false,true)));
    l.action="plant";
    assert(can_recover(recovery_proof(l,true,false,true,true,1100,true,true)));
    assert(!can_recover(recovery_proof(l,true,false,true,true,0,true,true)));
    assert(!can_recover(recovery_proof(l,true,false,true,true,1100,false,true)));
    assert(!can_recover(recovery_proof(l,true,false,true,false,0,false,false)));
    assert(same_ledger(l,l));Ledger other=l;other.sent++;assert(!same_ledger(l,other));
    assert(can_recover("no-request-created"));assert(!can_recover("server-rejected-review-required"));
    std::cout<<"PASS: planter recovery policy 29 checks\n";
}
