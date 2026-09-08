#pragma once
#include <sstream>
#include <string>
#include <cctype>

namespace pikmin::planter {
struct Ledger { long long sent{}; std::string action, seed; int slot{-1}; };
inline bool safe_token(const std::string &s, size_t max) {
    if(s.empty() || s.size()>max)return false;
    for(unsigned char c:s)if(!std::isalnum(c)&&c!='-'&&c!='_'&&c!=':')return false;
    return true;
}
inline bool parse_ledger(const std::string &raw, Ledger &l) {
    if(raw.size()>512)return false;
    std::istringstream in(raw);std::string extra;
    return bool(in>>l.sent>>l.action>>l.seed>>l.slot) && !(in>>extra) && l.sent>0 &&
        (l.action=="pluck"||l.action=="plant") && safe_token(l.seed,256) && l.slot>=0 && l.slot<8;
}
struct RecoveryRequest { std::string nonce; long long at{}, pid{}; Ledger expected; };
inline bool parse_recovery(const std::string &raw, RecoveryRequest &r) {
    if(raw.size()>1024)return false;
    std::istringstream in(raw);std::string schema,extra;
    if(!(in>>schema>>r.nonce>>r.pid>>r.at>>r.expected.sent>>r.expected.action>>r.expected.seed>>r.expected.slot) || in>>extra)return false;
    bool empty=r.expected.sent==0&&r.expected.action=="none"&&r.expected.seed=="none"&&r.expected.slot==-1;
    Ledger l;
    std::string value=std::to_string(r.expected.sent)+" "+r.expected.action+" "+r.expected.seed+" "+std::to_string(r.expected.slot);
    return schema=="v1"&&safe_token(r.nonce,64)&&r.pid>0&&r.at>0&&(empty||(parse_ledger(value,l)&&l.sent<=r.at));
}
inline bool fresh_recovery(const RecoveryRequest &r,long long pid,long long now) {
    return r.pid==pid && now>=r.at && now-r.at<=90000;
}
inline bool same_ledger(const Ledger &a,const Ledger &b) {
    return a.sent==b.sent&&a.action==b.action&&a.seed==b.seed&&a.slot==b.slot;
}
// Only existing, complete game inventory is evidence. Never treat an unchanged seed as failure.
inline std::string recovery_proof(const Ledger &l,bool complete,bool live_request,bool permanent,
        bool seed_exists,long long planted,bool slot_matches,bool referenced_anywhere) {
    if(live_request)return "rpc-in-flight";
    if(!complete)return "inventory-incomplete";
    if(!permanent)return "slot-unavailable";
    if(l.action=="plant" && seed_exists && planted>0 && slot_matches)return "plant-state-confirmed";
    if(l.action=="pluck" && !seed_exists && !referenced_anywhere)return "pluck-target-retired";
    return "result-unknown";
}
inline bool can_recover(const std::string &proof) {
    return proof=="plant-state-confirmed"||proof=="pluck-target-retired"||proof=="no-request-created";
}
}
