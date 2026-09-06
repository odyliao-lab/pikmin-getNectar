#pragma once
#include <cstdint>
#include <sstream>
#include <string>

namespace pikmin::planter {
constexpr unsigned default_steps = 2 | 4 | 8; // 1000 / 3000 / 5000 only.
inline unsigned step_bit(int required) {
    switch (required) {
        case 100: return 1; case 1000: return 2; case 3000: return 4;
        case 5000: return 8; case 10000: return 16;
        default: return required > 0 ? 32 : 0;
    }
}
inline unsigned parse_steps(const std::string &text) {
    if (text.size() > 128) return 0;
    std::istringstream input(text);
    std::string word;
    unsigned mask = 0;
    bool none = false;
    while (input >> word) {
        if (word == "none") { none = true; continue; }
        if (word == "100") mask |= 1;
        else if (word == "1000") mask |= 2;
        else if (word == "3000") mask |= 4;
        else if (word == "5000") mask |= 8;
        else if (word == "10000") mask |= 16;
        else if (word == "other") mask |= 32;
        else return 0; // Malformed/empty configuration never broadens selection.
    }
    return none ? 0 : mask;
}
inline bool step_allowed(unsigned mask, int required) { return (mask & step_bit(required)) != 0; }
// v152 PlanterSlot.Start selects permanentSlotPrefab for SlotType == 0.
// Also require an existing, bounded server slot; never create/expand slots.
inline bool permanent_slot(int type, int index, int max_slots) {
    return type == 0 && index >= 0 && index < max_slots && max_slots <= 8;
}
inline bool mature(int64_t planted, int steps, int required, bool native_ready) {
    return planted > 0 && required > 0 && steps >= required && native_ready;
}
inline bool can_plant(int64_t planted, bool in_slot, bool starred, int required,
                      bool special, bool has_point) {
    return planted == 0 && !in_slot && !starred && required > 0 && !special && has_point;
}
inline bool storage_room(int count, int capacity) {
    return count >= 0 && capacity > 0 && capacity <= 100000 && count < capacity;
}
} // namespace pikmin::planter
