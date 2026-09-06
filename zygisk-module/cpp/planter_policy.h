#pragma once
#include <cstdint>

namespace pikmin::planter {
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
