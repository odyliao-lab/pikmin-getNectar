#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
using SimpleMethod = void (*)(void *, void *);
long long clock_ms = 10000, planting_result_stop_requested_ms = 0;
std::string mode = "all";
long long now_ms() { return clock_ms; }
std::string read_return_mode() { return mode; }
int roots = 0, clicks = 0, reason_value = 13;
bool interactable = true, touch = true, loading = false, exception_click = false;
int overlay, common_obj, button_obj;
uint64_t new_root(void *, bool) { ++roots; return (1ULL << 40) + roots; }
void free_root(uint64_t handle) { assert(handle > (1ULL << 40)); --roots; }
auto gchandle_new = new_root;
auto gchandle_free = free_root;
struct ScopedManagedRoot {
    uint64_t h;
    ScopedManagedRoot(void *p, decltype(gchandle_new) n, decltype(gchandle_free)) : h(n(p, false)) {}
    ~ScopedManagedRoot() { free_root(h); }
};
void *object_get_class(void *p) { return p; }
void *class_get_method_from_name(void *, const char *name, int) { return const_cast<char *>(name); }
void *field_lookup(void *, const char *) { return &button_obj; }
void field_read(void *, void *, void *out) { *static_cast<void **>(out) = &button_obj; }
auto class_get_field = field_lookup;
auto field_get_value = field_read;
void *find_class(const char *, const char *) { return nullptr; }
void A64HookFunction(void *, void *, void **) {}
#define LOGI(...) ((void)0)
#include "../cpp/planting_reward.inc"
void *invoke(const void *method, void *, void **, void **ex) {
    auto name = static_cast<const char *>(method);
    if (!strcmp(name,"get_CommonItemDialog")) return &common_obj;
    if (!strcmp(name,"get_ReasonCase")) return &reason_value;
    if (!strcmp(name,"get_Interactable")) return &interactable;
    if (!strcmp(name,"WillReceiveTouch")) return &touch;
    if (!strcmp(name,"get_ShowLoadingVisuals")) return &loading;
    if (!strcmp(name,"InvokeClick")) { ++clicks; if (exception_click) *ex = &overlay; }
    return nullptr;
}
void *unbox(void *p) { return p; }
void arm() { hooked_reward_start(&overlay,nullptr); clock_ms += 1100; }
int main() {
    reward_invoke = invoke; reward_unbox = unbox;
    arm(); maybe_collect_planting_reward(); assert(clicks==1 && roots==0);
    maybe_collect_planting_reward(); assert(clicks==1); // no repeated click
    reason_value=7; arm(); maybe_collect_planting_reward(); assert(clicks==1 && roots==0);
    reason_value=13; mode="off"; arm(); assert(!reward_overlay && roots==0);
    mode="all"; arm(); mode="off"; maybe_collect_planting_reward(); assert(roots==0 && clicks==1);
    planting_result_stop_requested_ms=clock_ms; arm(); maybe_collect_planting_reward(); assert(clicks==2);
    planting_result_stop_requested_ms=0; mode="all";
    loading=true; arm(); maybe_collect_planting_reward(); assert(clicks==2 && roots==1);
    clock_ms+=61000; maybe_collect_planting_reward(); assert(roots==0);
    loading=false; interactable=false; arm(); maybe_collect_planting_reward(); assert(clicks==2);
    interactable=true; touch=false; clock_ms+=600; maybe_collect_planting_reward(); assert(clicks==2);
    touch=true; exception_click=true; clock_ms+=600; maybe_collect_planting_reward();
    assert(clicks==3 && roots==0); maybe_collect_planting_reward(); assert(clicks==3);
    arm(); arm(); assert(roots==1); forget_reward_overlay(); assert(roots==0);
    puts("PASS planting reward: scoped reason, authorization, readiness, expiry, no retry, roots (mock runtime only)");
}
