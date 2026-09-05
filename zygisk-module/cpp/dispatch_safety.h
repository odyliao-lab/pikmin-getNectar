#pragma once
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace pikmin {
inline bool batch_duration_safe(int64_t ms) { return ms > 0 && ms <= 2000; }
inline bool dispatch_status_available(int status) { return status == 1 || status == 32; }
inline bool gift_candidate_allowed(const std::string &designated, const std::string &candidate,
                                   bool native_allows) {
    return !designated.empty() && candidate == designated && native_allows;
}

inline std::vector<std::string> dispatch_ids(const std::string &csv) {
    std::vector<std::string> ids;
    std::set<std::string> unique;
    size_t begin = 0;
    while (begin < csv.size()) {
        const size_t end = csv.find(',', begin);
        const std::string id = csv.substr(begin, end == std::string::npos ? end : end - begin);
        if (id.empty() || id.find_first_of("\t\r\n<>") != std::string::npos ||
            !unique.insert(id).second || ids.size() >= 100) return {};
        ids.push_back(id);
        if (end == std::string::npos) return ids;
        begin = end + 1;
    }
    return {}; // empty input or trailing delimiter fails closed
}

inline bool same_dispatch_team(const std::vector<std::string> &a,
                               const std::vector<std::string> &b) {
    return !a.empty() && a.size() == b.size() &&
           std::set<std::string>(a.begin(), a.end()) == std::set<std::string>(b.begin(), b.end());
}

// Native-only IDs; no managed pointers/GC lifetime changes. A sent lease is
// not released by a timeout, RPC exception, mode change, or a new picker.
// Release requires a live busy->available transition, or a later complete
// task projection where the expedition is gone. Unsent selections can cancel.
class DispatchReservations {
    struct Hold { std::string owner; bool sent{}; bool saw_busy{}; int64_t sent_ms{}; };
    std::map<std::string, Hold> holds_;
public:
    bool has_sent(const std::string &owner) const {
        for (const auto &entry : holds_) if (entry.second.owner == owner && entry.second.sent) return true;
        return false;
    }
    bool permits(const std::string &id, const std::string &owner) const {
        const auto it = holds_.find(id);
        return it == holds_.end() || (it->second.owner == owner && !it->second.sent);
    }
    bool reserve(const std::string &owner, const std::vector<std::string> &ids) {
        if (owner.empty() || ids.empty() || has_sent(owner)) return false;
        const std::set<std::string> unique(ids.begin(), ids.end());
        if (unique.size() != ids.size() || unique.count("")) return false;
        for (const auto &id : ids) if (!permits(id, owner)) return false;
        cancel(owner);
        for (const auto &id : ids) holds_[id] = Hold{owner};
        return true;
    }
    void sent(const std::string &owner, int64_t now) {
        for (auto &entry : holds_) if (entry.second.owner == owner) {
            entry.second.sent = true;
            entry.second.sent_ms = now;
        }
    }
    void cancel(const std::string &owner) {
        for (auto it = holds_.begin(); it != holds_.end();) {
            if (it->second.owner == owner && !it->second.sent) it = holds_.erase(it);
            else ++it;
        }
    }
    void observe(const std::string &id, int status) {
        auto it = holds_.find(id);
        if (it == holds_.end() || !it->second.sent) return;
        if (status == 2) it->second.saw_busy = true;
        else if (dispatch_status_available(status) && it->second.saw_busy) holds_.erase(it);
    }
    void reconcile_tasks(const std::set<std::string> &live, int64_t observed_ms) {
        for (auto it = holds_.begin(); it != holds_.end();) {
            if (it->second.sent && observed_ms > it->second.sent_ms && !live.count(it->second.owner))
                it = holds_.erase(it);
            else ++it;
        }
    }
    size_t size() const { return holds_.size(); }
};
}
