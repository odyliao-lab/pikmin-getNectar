#pragma once
#include <cstdint>

namespace pikmin {
// v152 PikminTaskProto: Carry=1; CompletionRequirement.NONE=1,
// CALL_PREPARE_ACTION=2. Live v152 walking/mushroom Carry resources omit the
// requirement (0). The garden's Carry branch creates their collectable group
// without a requirement/time gate; this is NOT permission for other task kinds.
// Only extend the legacy due-time path for explicit carried resources. Never infer
// readiness for an active PoiChallenge, an unstarted expedition or unknown data.
inline bool collectable_carry_resource(int task_case, int requirement,
                                      bool has_resource, int64_t finish_ms, int64_t now) {
    return task_case == 1 && (requirement == 0 || requirement == 1) && has_resource &&
           now > 0 && finish_ms >= 0 && finish_ms <= now;
}
inline bool return_task_ready(int task_case, int requirement, bool has_resource,
                              int64_t finish_ms, int64_t now) {
    return (now > 0 && finish_ms > 0 && finish_ms <= now) ||
           collectable_carry_resource(task_case, requirement, has_resource, finish_ms, now);
}
// Unrelated removals can lower total task count while our task is still there.
inline bool return_confirmed(bool complete_snapshot, bool pending_seen) {
    return complete_snapshot && !pending_seen;
}
}
