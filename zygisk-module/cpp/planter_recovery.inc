// Included within planter_engine after Snapshot and helpers. No new managed calls.
constexpr const char *recovery_request_path="/data/local/tmp/pikmin-planter-recover.txt";
char recovery_state_path[512]{};
std::string recovery_nonce="none", recovery_result="ready", recovery_proof_seen;
long long recovery_seen_at{};
bool recovery_waiting{};
std::string read_bounded(const char *path,size_t limit) {
    std::ifstream in(path);std::string out;char c;
    while(in.get(c)){out+=c;if(out.size()>limit)return {};}
    return in.bad()?std::string():out;
}
void recovery_status() {
    atomic_file(recovery_state_path,"v1\t"+std::to_string(now_ms())+"\t"+std::to_string(getpid())+
            "\t"+recovery_nonce+"\t"+recovery_result+"\n");
}
void recover(const Snapshot &s) {
    using namespace pikmin::planter;
    RecoveryRequest request;
    if(!parse_recovery(read_bounded(recovery_request_path,1024),request)){
        if(recovery_waiting){recovery_result="invalid-request";recovery_waiting=false;recovery_seen_at=0;}
        recovery_status();return;
    }
    if(request.nonce==recovery_nonce&&!recovery_waiting){recovery_status();return;}
    if(request.nonce!=recovery_nonce){recovery_nonce=request.nonce;recovery_seen_at=0;recovery_proof_seen.clear();}
    recovery_waiting=false;
    auto reject=[&](const std::string &reason){recovery_result=reason;recovery_seen_at=0;recovery_status();};
    if(!fresh_recovery(request,getpid(),now_ms())){reject("expired-or-game-restarted");return;}
    if(pending.task){reject("rpc-in-flight");return;}
    if(!compatible){reject("incompatible");return;}
    if(!s.complete||s.capacity<=0||s.pikmins.empty()){
        recovery_seen_at=0;recovery_waiting=true;recovery_result="checking-inventory";recovery_status();return;
    }
    bool exists=access(pending_path,F_OK)==0;
    std::string raw=exists?read_bounded(pending_path,512):std::string(),proof;
    Ledger ledger;
    if(exists) {
        if(!parse_ledger(raw,ledger)){reject("invalid-ledger");return;}
        if(!same_ledger(ledger,request.expected)){reject("ledger-changed");return;}
        bool permanent=false,slot_matches=false,referenced=false;
        for(const auto &slot:s.slots) {
            if(slot.seed==ledger.seed)referenced=true;
            if(slot.index==ledger.slot){permanent=permanent_slot(slot.type,slot.index,s.max_slots);slot_matches=slot.seed==ledger.seed;}
        }
        auto seed=s.seeds.find(ledger.seed);
        proof=recovery_proof(ledger,true,false,permanent,seed!=s.seeds.end(),
                seed==s.seeds.end()?0:seed->second.planted,slot_matches,referenced);
    } else {
        if(request.expected.sent!=0||restart_blocked){reject("ledger-missing");return;}
        if(blocked.empty()){reject("not-paused");return;}
        proof=(blocked=="request-build-failed"||blocked=="audit-write-failed")?"no-request-created":"result-unknown";
    }
    if(!can_recover(proof)){reject(proof);return;}
    std::string proof_key=proof+raw;
    if(recovery_proof_seen!=proof_key||recovery_seen_at==0){
        recovery_proof_seen=proof_key;recovery_seen_at=now_ms();recovery_waiting=true;
        recovery_result="checking-inventory";recovery_status();return;
    }
    if(now_ms()-recovery_seen_at<5000){recovery_waiting=true;recovery_status();return;}
    if(exists) {
        // Recheck the exact pending bytes and archive before releasing the pause.
        if(read_bounded(pending_path,512)!=raw){reject("ledger-changed");return;}
        std::string archived=std::string(pending_path)+".recovered";
        if(std::rename(pending_path,archived.c_str())!=0){reject("archive-failed");return;}
    }
    history("manual-recovered",ledger.seed,ledger.slot,proof+" request="+request.nonce);
    restart_blocked=false;blocked.clear();next_action=now_ms()+5000;
    recovery_result="recovered-"+proof;recovery_status();
    // No RPC is submitted here. The next ordinary tick respects current mode and filters.
}
