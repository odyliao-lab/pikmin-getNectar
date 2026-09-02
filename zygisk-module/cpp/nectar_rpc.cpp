#include "hack.h"
#include "log.h"
#include "xdl.h"
#include "And64InlineHook.hpp"

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <dlfcn.h>
#include <fstream>
#include <ifaddrs.h>
#include <map>
#include <net/if.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <thread>
#include <time.h>
#include <unistd.h>

namespace {

// Pikmin Bloom 152.0 / arm64-v8a. Regenerated from the installed APK's
// libil2cpp.so plus global-metadata.dat; nonmatching binaries fail closed.
constexpr off_t kExpectedIl2CppSize = 254505288;
constexpr uintptr_t kRegisterMapObjectRva = 0x5A50188;
constexpr uintptr_t kMapObjectManagerUpdateRva = 0x5A51E7C;
constexpr uintptr_t kFlowerModelUpdatedRva = 0x5A6309C;
constexpr uintptr_t kRpcManagerConstructorRva = 0x727B96C;
constexpr uintptr_t kRpcManagerLoggedInRva = 0x727B25C;
constexpr uintptr_t kPlantingInitRva = 0x5F2A650;
constexpr uintptr_t kPlantingStartRva = 0x5F2A7C4;
constexpr uintptr_t kPlantingStateUpdatedRva = 0x5F2B000;
constexpr uintptr_t kLocationControllerAwakeRva = 0x71B4B90;
constexpr uintptr_t kRequestConstructorRva = 0x72181C8;
constexpr uintptr_t kRequestSetMapObjectIdRva = 0x72183C0;
constexpr uintptr_t kRequestSetIncludeFailureRva = 0x72184A0;
constexpr uintptr_t kSendClaimRva = 0x726A6D0;
constexpr uintptr_t kTaskIsCompletedRva = 0xC934858;
constexpr uintptr_t kTaskIsFaultedRva = 0xC93A26C;

constexpr size_t kInitialMapObjectProtoOffset = 0x68;
constexpr size_t kInteractionSettingsOffset = 0xA8;
constexpr size_t kLocationControllerOffset = 0x10;
constexpr size_t kRawLocationOffset = 0xA0;
constexpr size_t kPlantingSenderOffset = 0x120;
constexpr size_t kPlantingLocationControllerOffset = 0x48;
constexpr size_t kChangeSenderItemOffset = 0x20;
constexpr size_t kMapObjectIdOffset = 0x18;
constexpr size_t kMapObjectPointOffset = 0x20;
constexpr size_t kMapObjectPayloadOffset = 0x30;
constexpr size_t kPointLatitudeOffset = 0x18;
constexpr size_t kPointLongitudeOffset = 0x20;
constexpr size_t kFlowerStateOffset = 0x18;
constexpr size_t kFlowerRemainingOffset = 0x30;
constexpr size_t kFlowerWiltingTimeOffset = 0x38;
constexpr size_t kFlowerBloomedTimeOffset = 0x40;
constexpr size_t kFlowerRewardReceivedOffset = 0x48;

using ObjectGetClass = void *(*)(void *);
using ObjectGetVirtualMethod = void *(*)(void *, void *);
using ClassGetName = const char *(*)(void *);
using DomainGet = void *(*)();
using DomainGetAssemblies = const void **(*)(void *, size_t *);
using AssemblyGetImage = void *(*)(const void *);
using ClassFromName = void *(*)(void *, const char *, const char *);
using ClassGetMethodFromName = void *(*)(void *, const char *, int);
using ClassGetMethods = void *(*)(void *, void **);
using MethodGetName = const char *(*)(void *);
using MethodGetParamCount = uint32_t (*)(void *);
using ObjectNew = void *(*)(void *);
using ArrayNew = void *(*)(void *, size_t);
using StringNew = void *(*)(const char *);
using GcHandleNew = uint32_t (*)(void *, bool);
using TaskBool = bool (*)(void *, void *);
using RegisterMapObject = void (*)(void *, void *, int, void *);
using SimpleMethod = void (*)(void *, void *);
using GetFlowerProto = void *(*)(void *, void *);
using ProtoMethod = void (*)(void *, void *, void *);
using RpcManagerLoggedIn = void (*)(void *, void *, void *, void *, void *);
using RequestConstructor = void (*)(void *, void *);
using RequestSetString = void (*)(void *, void *, void *);
using RequestSetBool = void (*)(void *, bool, void *);
using SendClaim = void *(*)(void *, void *, void *, int, void *);
using CompletePikminTask = void *(*)(void *, void *, bool, void *);
using PreparePikminTasks = void *(*)(void *, void *, void *);
using ShouldPrepareCompletion = bool (*)(void *, void *, void *);
using GetInventoryItemId = void *(*)(void *, void *);
using GetPikminTaskList = void *(*)(void *, void *);
using GetPikminTaskProto = void *(*)(void *, void *);
using GetTaskFinishTimeMs = int64_t (*)(void *, void *);
using GetTaskLong = int64_t (*)(void *, void *);
using GetTaskDouble = double (*)(void *, void *);
using GetTaskVariant = void *(*)(void *, void *);
using GetTaskInt = int (*)(void *, void *);
using GetExpeditionDurationMs = int64_t (*)(void *, void *);
using GetExpeditionDataByIndex = void *(*)(void *, int, void *, void *);
using GetExpeditionBool = bool (*)(void *, void *);
using ExpeditionDataStoreConstructor = void (*)(void *, void *);
using GetEnumerable = void *(*)(void *, void *);
using EnumeratorMoveNext = bool (*)(void *, void *);
using EnumeratorCurrent = void *(*)(void *, void *);
using PickFastestPikmins = void *(*)(void *, void *, void *, void *, void *);
using SetExpeditionPikmins = void (*)(void *, void *, void *);
using StartExpedition = void *(*)(void *, void *);
using GetManagedString = void *(*)(void *, void *);
using StartPlantingWithConfirmation = void *(*)(void *, void *, bool, void *);
using StopPlantingWithConfirmation = void *(*)(void *, bool, void *);
using NoArgTask = void *(*)(void *, void *);
// Shape shared by any zero-arg managed getter that returns an object
// reference (System.Threading.Tasks.Task.Exception, Exception.Message, ...).
using ObjectGetter = void *(*)(void *, void *);
using PikminTaskPreparerConstructor = void (*)(void *, void *, void *, void *, void *);
using PikminTaskActionManagerConstructor = void (*)(void *, void *);

struct Il2CppStringLayout {
    void *klass;
    void *monitor;
    int32_t length;
    char16_t chars[1];
};

struct FlowerRecord {
    double latitude{};
    double longitude{};
    int state{};
    long long wilting_ms{};
    bool received{};
    bool attempted{};
};

ObjectGetClass object_get_class{};
ObjectGetVirtualMethod object_get_virtual_method{};
ClassGetName class_get_name{};
DomainGet domain_get{};
DomainGetAssemblies domain_get_assemblies{};
AssemblyGetImage assembly_get_image{};
ClassFromName class_from_name{};
ClassGetMethodFromName class_get_method_from_name{};
ClassGetMethods class_get_methods{};
MethodGetName method_get_name{};
MethodGetParamCount method_get_param_count{};
ObjectNew object_new{};
ArrayNew array_new{};
StringNew string_new{};
GcHandleNew gchandle_new{};
RegisterMapObject original_register_map_object{};
SimpleMethod original_map_update{};
GetFlowerProto original_get_flower_proto{};
ProtoMethod original_construct_proto_object{};
ProtoMethod original_update_proto_object{};
SimpleMethod original_flower_model_updated{};
SimpleMethod original_rpc_manager_constructor{};
RpcManagerLoggedIn original_rpc_manager_logged_in{};
SimpleMethod original_planting_init{};
SimpleMethod original_planting_start{};
SimpleMethod original_planting_state_updated{};
SimpleMethod original_location_controller_awake{};
RequestConstructor request_constructor{};
RequestSetString request_set_map_object_id{};
RequestSetBool request_set_include_failure{};
SendClaim send_claim{};
CompletePikminTask original_complete_pikmin_task{};
PreparePikminTasks original_prepare_pikmin_tasks{};
ShouldPrepareCompletion original_should_prepare_completion{};
GetInventoryItemId get_inventory_item_id{};
GetPikminTaskList original_get_pikmin_task_list{};
GetPikminTaskProto get_pikmin_task_proto{};
GetTaskFinishTimeMs get_task_finish_time_ms{};
GetTaskVariant get_task_finish_location{};
GetTaskVariant get_task_pikmin_id{};
GetTaskVariant get_task_carry{};
GetTaskVariant get_task_expedition{};
GetTaskVariant get_task_gift{};
GetTaskVariant get_task_poi_challenge{};
GetTaskVariant get_expedition_fruit{};
GetTaskVariant get_expedition_seed{};
GetTaskVariant get_expedition_gift{};
GetTaskVariant get_expedition_bloomed_poi{};
GetTaskVariant get_expedition_postcard{};
GetTaskVariant get_expedition_postcard_with_items{};
GetTaskVariant get_bloomed_poi_reward_v2{};
GetTaskVariant get_poi_reward_fruit{};
GetTaskVariant get_carry_resource{};
GetTaskVariant get_carry_pikmin_seed{};
GetTaskInt get_seed_type{};
GetTaskVariant get_seed_pikmin{};
GetTaskInt get_pikmin_type{};
GetTaskInt get_pikmin_category{};
GetTaskInt get_pikmin_full_asset{};
GetTaskInt get_gift_item_case{};
GetTaskVariant get_gift_deco{};
GetTaskVariant get_gift_rare_deco{};
GetTaskVariant get_deco_pikmin_type{};
GetTaskInt get_deco_box_type{};
GetTaskInt get_rare_deco_category{};
GetTaskInt get_task_case{};
GetTaskInt get_expedition_target_case{};
GetTaskLong get_expedition_expiration_time_ms{};
GetTaskVariant get_expedition_spawn_point{};
GetTaskDouble get_point_lat_degrees{};
GetTaskDouble get_point_lng_degrees{};
GetTaskInt get_resource_type{};
GetTaskInt get_resource_num_pikmins{};
GetTaskInt get_resource_honey_flower_kind{};
GetTaskInt get_resource_weight_grams{};
GetTaskVariant get_resource_flower_kind{};
GetTaskVariant get_resource_honey_balls{};
GetTaskInt get_honey_num_balls{};
GetTaskInt get_honey_type{};
GetTaskInt get_honey_flower_kind{};
GetTaskVariant get_honey_flower_kind_text{};
GetTaskVariant get_expedition_item_key{};
GetExpeditionDurationMs original_get_expedition_total_duration_ms{};
GetExpeditionDataByIndex get_expedition_data_by_index{};
GetExpeditionBool get_expedition_can_try_start{};
// Two more candidates from the INVALID_ARGUMENT investigation
// (HANDOFF_2026-08-31_RPC_DIAGNOSTICS.md): CanTryStart alone may not be the
// full precondition set the server checks, and the picked team's cached
// carrying-power/duration fields may be stale immediately after a GPS
// teleport if the game's own UI flow normally forces a recompute that this
// module's tighter selection-to-start window skips.
GetExpeditionBool get_expedition_has_enough_carrying_power{};
SimpleMethod invalidate_expedition_cached_values{};
ExpeditionDataStoreConstructor original_expedition_data_store_constructor{};
GetEnumerable get_pikmin_item_collection{};
PickFastestPikmins pick_fastest_pikmins{};
SetExpeditionPikmins set_expedition_pikmins{};
StartExpedition start_expedition{};
GetManagedString get_planting_flower_petal_id{};
StartPlantingWithConfirmation start_planting_with_confirmation{};
StopPlantingWithConfirmation stop_planting_with_confirmation{};
NoArgTask close_planting_result_dialog{};
SimpleMethod original_planting_result_dialog_start{};
PikminTaskPreparerConstructor original_pikmin_task_preparer_constructor{};
PikminTaskActionManagerConstructor original_pikmin_task_action_manager_constructor{};
TaskBool task_is_completed{};
TaskBool task_is_faulted{};
void *request_class{};
void *game_domain{};
bool runtime_metadata_ready{};
void *rpc_manager{};
void *planting_controller{};
void *interaction_settings{};
void *location_controller{};
void *return_preparer{};
void *return_inventory_manager{};
void *return_action_manager{};
void *expedition_data_store{};
char flower_log_path[512]{};
char mode_path[512]{};
char target_path[512]{};
char status_path[512]{};
char claim_log_path[512]{};
char system_gps_path[512]{};
// service.sh rewrites the system GPS file every two seconds, so anything older
// than this means its loop is not running and the contents cannot be trusted.
constexpr long long kSystemGpsMaxAgeSeconds = 15;
constexpr int kMaxPikminTaskListCount = 512;
long long last_stale_gps_age_logged = -1;
char return_trace_path[512]{};
char return_history_path[512]{};
char return_mode_path[512]{};
char return_postcard_policy_path[512]{};
char return_status_path[512]{};
char return_batch_limit_path[512]{};
char compatibility_path[512]{};
char dispatch_candidates_path[512]{};
char dispatch_status_path[512]{};
char dispatch_mode_path[512]{};
// Optional restriction for armed mode. Missing/unknown means all existing
// kinds remain eligible; the flower-farm controller explicitly writes farm.
char dispatch_kinds_path[512]{};
char dispatch_target_path[512]{};
char dispatch_ready_path[512]{};
char dispatch_history_path[512]{};
char dispatch_diagnostics_path[512]{};
// Forensic log for start_expedition() RPC faults -- see
// append_rpc_fault_diagnostics() and read_task_exception_message() below.
// Kept separate from dispatch_history.tsv so it never touches that file's
// existing column contract (the Java side parses it with a fixed split).
char dispatch_rpc_fault_path[512]{};
// Forensic log for a batch target that fails the dispatch gate without ever
// reaching start_expedition() -- see the "batch target blocked" LOGI site.
// Persisted (not just logged) because a full batch run can take many
// minutes and this needs to be readable after the fact, not only via a
// live logcat capture.
char dispatch_gate_block_path[512]{};
// Per-tick heartbeat while a batch target is set -- see the call site in
// write_dispatch_candidates() and append_dispatch_tick_trace() below.
char dispatch_tick_trace_path[512]{};
// This is deliberately a separate control plane from nectar collection and
// expedition dispatch. Missing/unknown values mean observe only: the module
// must never stop a planting session that the player started manually.
char planting_control_mode_path[512]{};
char planting_control_status_path[512]{};
std::string dispatch_confirmation_pending_id;
long long dispatch_confirmation_started_observed_ms{};
std::string dispatch_last_gift_skip_id;
// Batch mode deliberately owns one confirmation lock: its Java controller
// must not advance to the next GPS point until this exact task is settled.
// Armed mode is different.  A moving GPS provider can leave nearby targets
// behind before a sequential confirmation finishes, so it may start a small
// bounded group from one live task-list scan.  Keep every one of those calls
// rooted and independently visible until the inventory confirms it.
constexpr size_t kArmedMaxInFlight = 3;
constexpr size_t kArmedMaxStartsPerScan = 3;
struct ArmedDispatchInFlight {
    void *task{};
    uint32_t task_handle{};
    std::string kind;
    long long started_ms{};
    long long ms_since_previous_attempt{-1};
    bool rpc_result_recorded{};
    bool seen_this_scan{};
};
std::map<std::string, ArmedDispatchInFlight> armed_dispatches;
// Batch mode still has only one Task to watch.  This is deliberately not
// shared with armed_dispatches so batch cannot accidentally parallelise.
void *pending_expedition_task{};
uint32_t pending_expedition_task_handle{};
std::string pending_expedition_task_id;
std::string pending_expedition_task_kind;
long long pending_expedition_task_started_ms{};
// When the previous start_expedition() attempt (of any outcome) began, and
// how many faults have landed back-to-back -- both feed
// append_rpc_fault_diagnostics() so a rate-limit or streak pattern in the
// fault rate is visible without guessing at the game's internal reason.
long long last_expedition_attempt_started_ms{};
int consecutive_expedition_rpc_faults{};
long long pending_expedition_ms_since_previous_attempt = -1;
std::string observed_expedition_task_id;
int64_t observed_expedition_duration_ms{};
std::map<std::string, FlowerRecord> flowers;
std::map<std::string, std::string> last_flower_state;
long long last_tick_ms{};
long long last_claim_ms{};
long long last_status_ms{};
long long last_task_list_trace_ms{};
long long last_return_dry_run_ms{};
long long last_return_scheduler_ms{};
long long last_expedition_duration_observed_ms{};
bool test_once_sent{};
bool target_loaded{};
bool return_one_dispatched{};
bool return_one_waiting{};
int return_one_baseline_count{};
bool return_batch_waiting{};
bool return_batch_stopped{};
bool return_all_empty_reported{};
int return_batch_baseline_count{};
int return_batch_completed{};
long long return_batch_dispatched_ms{};
std::string last_return_batch_mode;
std::string return_batch_pending_id;
std::string return_batch_pending_reward;
bool return_reward_variant_metadata_logged[4]{};
bool return_expedition_target_metadata_logged[6]{};
bool return_bloomed_poi_reward_metadata_logged{};
bool return_bloomed_poi_fruit_metadata_logged{};
bool return_bloomed_poi_fruit_entry_metadata_logged{};
bool dispatch_enumerable_metadata_logged{};
bool gift_pikmin_metadata_logged{};
bool gift_target_metadata_logged{};
// First step for the flower-farm controller: enumerate the actual v152
// PlantingController methods once, without invoking any of them.  Starting or
// stopping planting will not be attempted until this passive probe identifies
// the game's real method contract.
bool planting_controller_metadata_logged{};
bool planting_control_owned{};
bool planting_control_start_attempted{};
bool planting_control_stop_attempted{};
std::string planting_control_last_mode = "observe";
std::string planting_control_last_action = "observe";
void *planting_control_pending_task{};
uint32_t planting_control_pending_task_handle{};
void *planting_result_dialog{};
long long planting_result_dialog_seen_ms{};
long long planting_result_stop_requested_ms{};
long long planting_result_close_after_ms{};
bool planting_result_auto_close_pending{};
bool planting_result_hook_installed{};
// Metadata-only, zero-invocation-risk probe for whether ExpeditionItemData
// exposes a preparation/validation method (Lock*, Prepare*, Validate*, ...)
// that the game's own UI might call before StartExpeditionAsync and that
// this module currently skips -- see the INVALID_ARGUMENT investigation in
// HANDOFF_2026-08-31_RPC_DIAGNOSTICS.md. Logged once per process, from the
// real runtime class via object_get_class() rather than the static
// find_class() lookup, so it reflects the exact instance in play.
bool expedition_item_metadata_logged{};
bool expedition_utils_metadata_logged{};
void *pending_task{};
uint32_t pending_task_handle{};
std::string pending_id;
double pending_gps_lat{};
double pending_gps_lng{};
double pending_distance{};
std::string last_result = "none";

long long now_ms();
void *find_class(const char *namespaze, const char *name);
void log_task_variant_metadata(void *proto);
bool current_location(double &latitude, double &longitude);
double distance_metres(double lat1, double lng1, double lat2, double lng2);
std::string read_dispatch_mode();
std::string read_dispatch_kind_filter();
std::string read_planting_control_mode();
std::string read_dispatch_target();
bool dispatch_target_is_ready(const std::string &target_id, long long observed_ms);
void append_dispatch_history(const char *event, const char *kind, const std::string &task_id,
                             int64_t duration_ms, int picked_count);
void log_class_methods(const char *label, void *proto_class);
void append_dispatch_gate_block(const std::string &task_id, const char *kind, bool lock_held,
                                bool requested, bool has_data, bool has_picked, int picked_count,
                                double distance, double allowed_distance);
void append_dispatch_tick_trace(const std::string &batch_target, bool target_seen_raw,
                                int raw_count, int filtered_count);
void maybe_return_tasks();

std::string utf8_string(void *value) {
    if (!value) return {};
    const auto *text = static_cast<Il2CppStringLayout *>(value);
    if (text->length < 1 || text->length > 256) return {};
    std::string result;
    result.reserve(static_cast<size_t>(text->length));
    for (int index = 0; index < text->length; ++index) {
        const char16_t c = text->chars[index];
        if (c < 0x80) result.push_back(static_cast<char>(c));
        else result.push_back('?');
    }
    return result;
}

void append_return_trace(const char *event, void *self, void *task_id) {
    const std::string id = utf8_string(task_id);
    LOGI("[RETURN-DIAG] %s self=%p taskId=%s", event, self, id.c_str());
    FILE *file = std::fopen(return_trace_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%p\t%s\n", now_ms(), event, self, id.c_str());
    std::fclose(file);
    chmod(return_trace_path, 0644);
}

// Append-only, controller-visible history. The status file below intentionally
// holds only the latest state; this file is the audit record shown in the APK.
void append_return_history(const char *event, int task_count, int completed,
                           bool waiting, bool discard_postcard) {
    FILE *file = std::fopen(return_history_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%s\t%d\t%d\t%d\t%s\t%s\t%s\n", now_ms(), event,
                 return_batch_pending_id.c_str(), task_count, completed, waiting ? 1 : 0,
                 discard_postcard ? "discard" : "keep", last_return_batch_mode.c_str(),
                 return_batch_pending_reward.c_str());
    std::fclose(file);
    chmod(return_history_path, 0644);
}

bool task_list_contains_id(void *items, int count, const std::string &target_id) {
    if (!items || target_id.empty()) return false;
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!task || !get_inventory_item_id) continue;
        void *task_id = get_inventory_item_id(task, nullptr);
        if (utf8_string(task_id) == target_id) return true;
    }
    return false;
}

const char *fruit_type_name(int type) {
    switch (type) {
        case 3: return "strawberry"; case 4: return "orange"; case 5: return "cherry";
        case 6: return "melon"; case 8: return "peach"; case 27: return "lemon";
        case 28: return "blueberry"; case 31: return "watermelon"; case 32: return "lime";
        case 33: return "muscat-one"; case 34: return "kiwi"; case 35: return "green-apple";
        case 36: return "muscat"; case 37: return "kumquat"; case 38: return "banana";
        case 39: return "grapefruit"; case 40: return "apple"; case 41: return "prune";
        case 42: return "grapes"; case 43: return "happy-fruit"; case 44: return "cut-kiwi";
        case 45: return "fig"; case 46: return "loquat"; case 47: return "mangosteen";
        default: return "unknown-fruit";
    }
}

const char *flower_kind_name(int kind) {
    switch (kind) {
        case 1: return "sunflower"; case 2: return "tulip"; case 3: return "pansy";
        case 4: return "rose"; case 5: return "common"; case 6: return "poinsettia";
        case 7: return "camellia"; case 8: return "plumblossom"; case 9: return "narcissus";
        case 10: return "cherryblossom"; case 11: return "nemophila"; case 12: return "carnation";
        case 13: return "callalily"; case 14: return "hydrangea"; case 15: return "lilium";
        case 16: return "hibiscus"; case 17: return "plumeria"; case 18: return "clusteramaryllis";
        case 19: return "cosmos"; case 20: return "cyclamen"; case 21: return "anemone";
        case 22: return "dianthus"; case 23: return "gentian"; case 24: return "chrysanthemum";
        case 25: return "hellebore"; case 26: return "cattleya"; case 27: return "hyacinth";
        case 28: return "sweetpea"; case 29: return "convallaria"; case 30: return "peony";
        case 31: return "nymphaea"; case 32: return "morningglory"; case 33: return "bougainvillea";
        case 34: return "dahlia"; case 35: return "clematis"; case 36: return "snowdrop";
        case 37: return "freesia"; case 38: return "canola"; case 39: return "rose2025";
        case 40: return "iris"; case 41: return "strelitzia"; case 42: return "celosia";
        case 43: return "marigold"; case 44: return "salvia"; case 45: return "primula";
        case 46: return "mothorchid"; case 47: return "snapdragon"; case 48: return "petunia";
        case 49: return "tulip2026"; case 50: return "forgetmenot"; case 51: return "poppy";
        case 52: return "bellflower"; case 53: return "epiphyllum"; case 54: return "canna";
        case 55: return "lisianthus"; case 56: return "thistle"; case 57: return "geranium";
        case 58: return "oxalis"; default: return "unknown-flower";
    }
}

const char *honey_type_name(int type) {
    switch (type) {
        case 1: return "white";
        case 2: return "red";
        case 3: return "blue";
        case 4: return "yellow";
        case 5: return "happy";
        default: return "unknown";
    }
}

// ResourceProto.WeightGrams is task weight, not the awarded nectar amount.
// The actual reward is the repeated HoneyBallProto.NumBalls payload.
std::string describe_resource_honey(void *resource) {
    if (!resource || !get_resource_honey_balls || !get_honey_num_balls || !get_honey_type)
        return {};
    void *repeated = get_resource_honey_balls(resource, nullptr);
    if (!repeated) return {};
    auto *repeated_bytes = static_cast<uint8_t *>(repeated);
    void *items = *reinterpret_cast<void **>(repeated_bytes + 0x10);
    const int count = *reinterpret_cast<int *>(repeated_bytes + 0x18);
    if (!items || count < 1 || count > 16) return {};
    std::string result;
    for (int index = 0; index < count; ++index) {
        void *honey = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!honey) continue;
        std::string flower = get_honey_flower_kind_text
                ? utf8_string(get_honey_flower_kind_text(honey, nullptr)) : "";
        if (flower.empty() && get_honey_flower_kind) {
            const int kind = get_honey_flower_kind(honey, nullptr);
            if (kind > 0) flower = flower_kind_name(kind);
        }
        if (flower.empty()) flower = "unknown-flower";
        if (!result.empty()) result += ";";
        result += flower;
        result += ":" + std::string(honey_type_name(get_honey_type(honey, nullptr)));
        result += ":" + std::to_string(get_honey_num_balls(honey, nullptr));
    }
    return result;
}

// All reward attributes below are read through v151 public protobuf getters.
// This avoids relying on private object offsets for the direct Fruit target.
std::string describe_resource(void *resource) {
    if (!resource || !get_resource_type || !get_resource_num_pikmins || !get_resource_weight_grams)
        return "unknown-fruit";
    const int type = get_resource_type(resource, nullptr);
    const int pikmins = get_resource_num_pikmins(resource, nullptr);
    std::string flower = get_resource_flower_kind
            ? utf8_string(get_resource_flower_kind(resource, nullptr)) : "";
    if (flower.empty() && get_resource_honey_flower_kind) {
        const int kind = get_resource_honey_flower_kind(resource, nullptr);
        if (kind > 0) flower = flower_kind_name(kind);
    }
    std::string result = std::string(fruit_type_name(type)) + " x" + std::to_string(pikmins);
    const std::string honey = describe_resource_honey(resource);
    if (!honey.empty()) result += " honey=" + honey;
    else if (!flower.empty()) result += " flower=" + flower;
    return result;
}

// Seed and gift targets contain their reward description before completion.
// Record only public protobuf fields so the history can be displayed without
// guessing from the post-completion inventory delta.
std::string describe_seed(void *seed) {
    if (!seed) return "seed:unavailable";
    const int seed_type = get_seed_type ? get_seed_type(seed, nullptr) : 0;
    void *pikmin = get_seed_pikmin ? get_seed_pikmin(seed, nullptr) : nullptr;
    const int pikmin_type = pikmin && get_pikmin_type ? get_pikmin_type(pikmin, nullptr) : 0;
    const int category = pikmin && get_pikmin_category ? get_pikmin_category(pikmin, nullptr) : 0;
    const int asset = pikmin && get_pikmin_full_asset ? get_pikmin_full_asset(pikmin, nullptr) : 0;
    return "seed:" + std::to_string(seed_type) + ":pikmin:" + std::to_string(pikmin_type)
            + ":category:" + std::to_string(category) + ":asset:" + std::to_string(asset);
}

std::string describe_gift(void *gift) {
    if (!gift) return "gift:unavailable";
    const int item_case = get_gift_item_case ? get_gift_item_case(gift, nullptr) : 0;
    void *deco = get_gift_deco ? get_gift_deco(gift, nullptr) : nullptr;
    if (deco) {
        void *pikmin = get_deco_pikmin_type ? get_deco_pikmin_type(deco, nullptr) : nullptr;
        const int pikmin_type = pikmin && get_pikmin_type ? get_pikmin_type(pikmin, nullptr) : 0;
        const int category = pikmin && get_pikmin_category ? get_pikmin_category(pikmin, nullptr) : 0;
        const int box_type = get_deco_box_type ? get_deco_box_type(deco, nullptr) : 0;
        return "gift:deco:pikmin:" + std::to_string(pikmin_type) + ":category:"
                + std::to_string(category) + ":box:" + std::to_string(box_type);
    }
    void *rare_deco = get_gift_rare_deco ? get_gift_rare_deco(gift, nullptr) : nullptr;
    if (rare_deco) {
        const int category = get_rare_deco_category ? get_rare_deco_category(rare_deco, nullptr) : 0;
        return "gift:rare:category:" + std::to_string(category);
    }
    return "gift:itemcase:" + std::to_string(item_case);
}

std::string describe_return_reward(void *task) {
    if (!task || !get_pikmin_task_proto) return "task-reward-unavailable";
    void *proto = get_pikmin_task_proto(task, nullptr);
    if (!proto) return "task-proto-unavailable";
    log_task_variant_metadata(proto);
    void *carry = proto && get_task_carry ? get_task_carry(proto, nullptr) : nullptr;
    if (carry) {
        void *seed = get_carry_pikmin_seed ? get_carry_pikmin_seed(carry, nullptr) : nullptr;
        if (seed) return "carry:" + describe_seed(seed);
        if (get_carry_resource && get_carry_resource(carry, nullptr)) return "carry:resource";
        return "carry:unknown";
    }
    void *expedition = get_task_expedition ? get_task_expedition(proto, nullptr) : nullptr;
    if (!expedition) {
        if (get_task_gift && get_task_gift(proto, nullptr)) return "gift-task:reward-not-in-task-proto";
        if (get_task_poi_challenge && get_task_poi_challenge(proto, nullptr)) return "poi-challenge:reward-in-completion-response";
        const int task_case = get_task_case ? get_task_case(proto, nullptr) : -1;
        return "taskcase:" + std::to_string(task_case) + ":unparsed";
    }
    if (get_expedition_seed) {
        void *seed = get_expedition_seed(expedition, nullptr);
        if (seed) return "expedition:" + describe_seed(seed);
    }
    if (get_expedition_fruit) {
        void *fruit = get_expedition_fruit(expedition, nullptr);
        if (fruit) return "expedition:fruit:" + describe_resource(fruit);
    }
    if (get_expedition_gift) {
        void *gift = get_expedition_gift(expedition, nullptr);
        if (gift) return "expedition:" + describe_gift(gift);
    }
    if (get_expedition_postcard && get_expedition_postcard(expedition, nullptr))
        return "expedition:postcard";
    if (get_expedition_postcard_with_items && get_expedition_postcard_with_items(expedition, nullptr))
        return "expedition:postcard-with-items:reward-not-in-task-proto";
    if (!get_expedition_bloomed_poi || !get_bloomed_poi_reward_v2 || !get_poi_reward_fruit)
        return "expedition:target-unavailable";
    void *bloomed = get_expedition_bloomed_poi(expedition, nullptr);
    if (!bloomed) {
        const int target_case = get_expedition_target_case ? get_expedition_target_case(expedition, nullptr) : -1;
        return "expedition:targetcase:" + std::to_string(target_case) + ":unparsed";
    }
    void *reward = get_bloomed_poi_reward_v2(bloomed, nullptr);
    void *repeated = reward ? get_poi_reward_fruit(reward, nullptr) : nullptr;
    if (!repeated) return "bloomed-poi:no-fruit";
    auto *repeated_bytes = static_cast<uint8_t *>(repeated);
    void *items = *reinterpret_cast<void **>(repeated_bytes + 0x10);
    const int count = *reinterpret_cast<int *>(repeated_bytes + 0x18);
    if (!items || count < 1 || count > 32) return "bloomed-poi:fruit-list-empty";
    std::string result = "bloomed-poi:";
    for (int index = 0; index < count; ++index) {
        void *resource = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!resource) continue;
        if (result.size() > 12) result += ",";
        result += describe_resource(resource);
    }
    return result == "bloomed-poi:" ? "bloomed-poi:fruit-list-empty" : result;
}

// Compact, replace-in-place status for the controller.  Unlike a dispatch
// trace, "batch-confirmed" is emitted only after the live inventory count
// decreases, so the APK can distinguish a request from a completed claim.
void write_return_status(const char *event, int task_count, int completed, bool waiting, bool discard_postcard) {
    FILE *file = std::fopen(return_status_path, "w");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%d\t%d\t%d\t%s\n", now_ms(), event, task_count,
                 completed, waiting ? 1 : 0, discard_postcard ? "discard" : "keep");
    std::fclose(file);
    chmod(return_status_path, 0644);
    append_return_history(event, task_count, completed, waiting, discard_postcard);
}

void write_compatibility_status(bool compatible, off_t observed_size) {
    FILE *file = std::fopen(compatibility_path, "w");
    if (!file) return;
    std::fprintf(file, "v152\t%s\t%lld\t%lld\n", compatible ? "compatible" : "incompatible",
                 static_cast<long long>(kExpectedIl2CppSize), static_cast<long long>(observed_size));
    std::fclose(file);
    chmod(compatibility_path, 0644);
}

// Conservative phase-one expedition probe.  It intentionally reuses only
// protobuf accessors that are already exercised by the return-reward engine:
// task Proto, FinishTimeMs, Expedition and TargetCase.  No new game method is
// resolved or hooked here, so the first live validation is limited to safely
// identifying unstarted Seed/Fruit candidates.
int count_enumerable_items(void *enumerable) {
    if (!enumerable || !object_get_class || !class_get_method_from_name) return 0;
    void *enumerable_class = object_get_class(enumerable);
    void *get_enumerator_method = enumerable_class
            ? class_get_method_from_name(enumerable_class, "GetEnumerator", 0) : nullptr;
    // Compiler-generated iterator types implement GetEnumerator explicitly.
    // Resolve it through the non-generic IEnumerable interface so that an
    // iterator in its initial (-2) state is cloned into a usable enumerator.
    if (!get_enumerator_method && object_get_virtual_method) {
        void *ienumerable = find_class("System.Collections", "IEnumerable");
        void *interface_get_enumerator = ienumerable
                ? class_get_method_from_name(ienumerable, "GetEnumerator", 0) : nullptr;
        get_enumerator_method = interface_get_enumerator
                ? object_get_virtual_method(enumerable, interface_get_enumerator) : nullptr;
    }
    auto get_enumerator = get_enumerator_method
            ? reinterpret_cast<GetEnumerable>(*reinterpret_cast<void **>(get_enumerator_method)) : nullptr;
    // Iterator blocks generated by IL2CPP can expose IEnumerator only via an
    // explicit interface implementation, so no public GetEnumerator appears
    // in the concrete class metadata. In that case the returned object itself
    // is already the IEnumerator.
    void *enumerator = get_enumerator ? get_enumerator(enumerable, nullptr) : enumerable;
    void *enumerator_class = enumerator && object_get_class ? object_get_class(enumerator) : nullptr;
    void *move_next_method = enumerator_class
            ? class_get_method_from_name(enumerator_class, "MoveNext", 0) : nullptr;
    if (!move_next_method && enumerator_class) {
        move_next_method = class_get_method_from_name(enumerator_class,
                "System.Collections.IEnumerator.MoveNext", 0);
    }
    if (!move_next_method && object_get_virtual_method) {
        void *ienumerator = find_class("System.Collections", "IEnumerator");
        void *interface_move_next = ienumerator
                ? class_get_method_from_name(ienumerator, "MoveNext", 0) : nullptr;
        move_next_method = interface_move_next
                ? object_get_virtual_method(enumerator, interface_move_next) : nullptr;
    }
    auto move_next = move_next_method
            ? reinterpret_cast<EnumeratorMoveNext>(*reinterpret_cast<void **>(move_next_method)) : nullptr;
    if (!move_next) {
        LOGI("[DISPATCH-OBSERVE] iterator unresolved object=%p getEnumerator=%p enumerator=%p moveNext=%p",
             enumerable, get_enumerator_method, enumerator, move_next_method);
        return 0;
    }
    int count{};
    while (count < 100 && move_next(enumerator, nullptr)) ++count;
    LOGI("[DISPATCH-OBSERVE] iterator counted object=%p enumerator=%p count=%d", enumerable, enumerator, count);
    return count;
}

// The game's SetPikmins API accepts IEnumerable<string>.  A managed string[]
// implements that interface, so it avoids fabricating a generic List<T> and
// keeps the hand-off entirely inside the game's managed runtime.
void *picked_pikmin_ids_array(void *enumerable) {
    if (!enumerable || !object_get_class || !class_get_method_from_name ||
        !object_get_virtual_method || !get_inventory_item_id || !array_new) return nullptr;
    void *ienumerable = find_class("System.Collections", "IEnumerable");
    void *ienumerator = find_class("System.Collections", "IEnumerator");
    void *string_class = find_class("System", "String");
    void *get_method = ienumerable ? class_get_method_from_name(ienumerable, "GetEnumerator", 0) : nullptr;
    void *move_method = ienumerator ? class_get_method_from_name(ienumerator, "MoveNext", 0) : nullptr;
    void *current_method = ienumerator ? class_get_method_from_name(ienumerator, "get_Current", 0) : nullptr;
    void *get_impl = get_method ? object_get_virtual_method(enumerable, get_method) : nullptr;
    auto get_enumerator = get_impl ? reinterpret_cast<GetEnumerable>(*reinterpret_cast<void **>(get_impl)) : nullptr;
    void *enumerator = get_enumerator ? get_enumerator(enumerable, nullptr) : nullptr;
    void *move_impl = enumerator && move_method ? object_get_virtual_method(enumerator, move_method) : nullptr;
    void *current_impl = enumerator && current_method ? object_get_virtual_method(enumerator, current_method) : nullptr;
    auto move_next = move_impl ? reinterpret_cast<EnumeratorMoveNext>(*reinterpret_cast<void **>(move_impl)) : nullptr;
    auto current = current_impl ? reinterpret_cast<EnumeratorCurrent>(*reinterpret_cast<void **>(current_impl)) : nullptr;
    if (!enumerator || !move_next || !current || !string_class) {
        LOGI("[DISPATCH-OBSERVE] ids unresolved enumerable=%p enumerator=%p get=%p move=%p current=%p",
             enumerable, enumerator, get_impl, move_impl, current_impl);
        return nullptr;
    }
    std::vector<void *> ids;
    while (ids.size() < 100 && move_next(enumerator, nullptr)) {
        void *pikmin = current(enumerator, nullptr);
        void *id = pikmin ? get_inventory_item_id(pikmin, nullptr) : nullptr;
        if (id) ids.push_back(id);
    }
    if (ids.empty()) return nullptr;
    void *array = array_new(string_class, ids.size());
    if (!array) return nullptr;
    auto **values = reinterpret_cast<void **>(static_cast<uint8_t *>(array) + 0x20);
    for (size_t index = 0; index < ids.size(); ++index) values[index] = ids[index];
    LOGI("[DISPATCH-OBSERVE] selected %zu Pikmin ids array=%p", ids.size(), array);
    return array;
}

void log_dispatch_enumerable_metadata(void *object, const char *label) {
    if (!object || !object_get_class || !class_get_methods || !method_get_name || dispatch_enumerable_metadata_logged) return;
    dispatch_enumerable_metadata_logged = true;
    void *klass = object_get_class(object);
    LOGI("[DISPATCH-META] %s object=%p class=%s", label, object,
         klass && class_get_name ? class_get_name(klass) : "?");
    void *iter{};
    for (int count{}; klass && count < 80 && (iter = class_get_methods(klass, &iter)); ++count) {
        const char *name = method_get_name(iter);
        LOGI("[DISPATCH-META] method=%s params=%u", name ? name : "?",
             method_get_param_count ? method_get_param_count(iter) : 0);
    }
}

void write_dispatch_candidates(void *list, long long observed_ms) {
    if (!list || !get_pikmin_task_proto || !get_task_finish_time_ms ||
        !get_task_expedition || !get_expedition_target_case) return;
    auto *bytes = static_cast<uint8_t *>(list);
    void *items = *reinterpret_cast<void **>(bytes + 0x10);
    const int count = *reinterpret_cast<int *>(bytes + 0x18);
    if (!items || count < 0 || count > kMaxPikminTaskListCount) return;
    FILE *file = std::fopen(dispatch_candidates_path, "w");
    if (!file) return;
    double current_latitude{}, current_longitude{};
    const bool has_current_location = current_location(current_latitude, current_longitude);
    const std::string dispatch_mode = read_dispatch_mode();
    const bool armed = dispatch_mode == "armed";
    const bool batch = dispatch_mode == "batch";
    const std::string armed_kind_filter = armed ? read_dispatch_kind_filter() : "all";
    // A controller stop is an explicit safety release.  Do not let a failed
    // or timed-out prior batch leave an in-memory confirmation lock that
    // blocks every later, independently armed batch.
    if (!armed && !batch && !dispatch_confirmation_pending_id.empty()) {
        dispatch_confirmation_pending_id.clear();
        dispatch_confirmation_started_observed_ms = 0;
    }
    // Turning armed mode off is an explicit controller release.  Do not keep
    // stale in-memory ids across a later, independent armed run.  The rooted
    // managed Tasks are intentionally not freed here: freeing a just-finished
    // Task has raced an IL2CPP continuation on v150.
    if (!armed && !armed_dispatches.empty()) armed_dispatches.clear();
    const std::string batch_target = batch ? read_dispatch_target() : "";
    const bool batch_ready = batch && !batch_target.empty() &&
            dispatch_target_is_ready(batch_target, observed_ms);
    const bool confirmation_due = !dispatch_confirmation_pending_id.empty() &&
            dispatch_confirmation_started_observed_ms < observed_ms;
    int candidates{};
    int expedition_tasks{};
    int finished_expedition_tasks{};
    int direct_seed_targets{}, direct_fruit_targets{}, gift_targets{}, bloomed_poi_targets{}, other_targets{};
    std::map<int, int> target_cases;
    std::map<int, int> unfinished_target_cases;
    int selections_applied{};
    int starts_requested{};
    bool start_requested{};
    bool pending_confirmation_seen{};
    // Whether this tick's raw list (before any of the continue-filters
    // below) contained the batch's named target at all -- see the tick
    // heartbeat appended after the loop. Distinguishes "the native tick
    // loop stopped running" from "it ran, but this candidate was
    // temporarily missing from what the game handed back", which look
    // identical from the Java side (both show as a silent stretch ending
    // in game-refresh-timeout).
    bool batch_target_seen_raw{};
    for (auto &entry : armed_dispatches) entry.second.seen_this_scan = false;
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        void *proto = task ? get_pikmin_task_proto(task, nullptr) : nullptr;
        void *expedition = proto ? get_task_expedition(proto, nullptr) : nullptr;
        const int target_case = expedition ? get_expedition_target_case(expedition, nullptr) : 0;
        const int64_t finish_ms = proto ? get_task_finish_time_ms(proto, nullptr) : 0;
        void *seed_target = expedition && get_expedition_seed ? get_expedition_seed(expedition, nullptr) : nullptr;
        void *fruit_target = expedition && get_expedition_fruit ? get_expedition_fruit(expedition, nullptr) : nullptr;
        void *bloomed_poi_target = expedition && get_expedition_bloomed_poi ? get_expedition_bloomed_poi(expedition, nullptr) : nullptr;
        void *gift_target = expedition && get_expedition_gift ? get_expedition_gift(expedition, nullptr) : nullptr;
        if (expedition) {
            ++expedition_tasks;
            ++target_cases[target_case];
            if (finish_ms != 0) ++finished_expedition_tasks;
            else ++unfinished_target_cases[target_case];
            if (seed_target) ++direct_seed_targets;
            else if (fruit_target) ++direct_fruit_targets;
            else if (get_expedition_gift && get_expedition_gift(expedition, nullptr)) ++gift_targets;
            else if (get_expedition_bloomed_poi && get_expedition_bloomed_poi(expedition, nullptr)) ++bloomed_poi_targets;
            else ++other_targets;
        }
        // The visible fruit list contains both direct Fruit targets and
        // BloomedPoi targets whose reward is a fruit collection.
        const char *kind = seed_target ? "seed" : (fruit_target || bloomed_poi_target ? "fruit" : (gift_target ? "gift" : nullptr));
        if (!task || !expedition || !kind) continue;
        void *id = get_inventory_item_id ? get_inventory_item_id(task, nullptr) : nullptr;
        const std::string id_text = utf8_string(id);
        if (batch && !batch_target.empty() && id_text == batch_target) batch_target_seen_raw = true;
        if (confirmation_due && id_text == dispatch_confirmation_pending_id)
            pending_confirmation_seen = true;
        auto armed_inflight = armed_dispatches.find(id_text);
        if (armed_inflight != armed_dispatches.end()) armed_inflight->second.seen_this_scan = true;
        if (finish_ms != 0) {
            if (confirmation_due && id_text == dispatch_confirmation_pending_id) {
                append_dispatch_history("inventory-confirmed", kind, id_text, 0, 0);
                dispatch_confirmation_pending_id.clear();
                dispatch_confirmation_started_observed_ms = 0;
            }
            if (armed_inflight != armed_dispatches.end()) {
                append_dispatch_history("inventory-confirmed", kind, id_text, 0, 0);
                armed_dispatches.erase(armed_inflight);
            }
            continue;
        }
        const int64_t expiration_ms = get_expedition_expiration_time_ms
                ? get_expedition_expiration_time_ms(expedition, nullptr) : 0;
        void *point = get_expedition_spawn_point ? get_expedition_spawn_point(expedition, nullptr) : nullptr;
        const double latitude = point && get_point_lat_degrees ? get_point_lat_degrees(point, nullptr) : 0.0;
        const double longitude = point && get_point_lng_degrees ? get_point_lng_degrees(point, nullptr) : 0.0;
        void *finish_point = proto && get_task_finish_location ? get_task_finish_location(proto, nullptr) : nullptr;
        const double finish_latitude = finish_point && get_point_lat_degrees
                ? get_point_lat_degrees(finish_point, nullptr) : 0.0;
        const double finish_longitude = finish_point && get_point_lng_degrees
                ? get_point_lng_degrees(finish_point, nullptr) : 0.0;
        const double distance = has_current_location && (latitude != 0.0 || longitude != 0.0)
                ? distance_metres(current_latitude, current_longitude, latitude, longitude) : -1.0;
        void *data = expedition_data_store && get_expedition_data_by_index
                ? get_expedition_data_by_index(expedition_data_store, 0, id, nullptr) : nullptr;
        if (data && !expedition_item_metadata_logged && object_get_class) {
            expedition_item_metadata_logged = true;
            log_class_methods("ExpeditionItemData", object_get_class(data));
        }
        // Force a recompute of this item's cached fields (duration,
        // CanTryStart, carrying power, ...) before reading any of them.
        // Candidate fix for the INVALID_ARGUMENT investigation: after a GPS
        // teleport, these caches may only refresh when the game's own UI
        // flow touches them, which this module's tighter
        // selection-to-start window otherwise skips.
        if (data && invalidate_expedition_cached_values) invalidate_expedition_cached_values(data, nullptr);
        int64_t game_duration = data && original_get_expedition_total_duration_ms
                ? original_get_expedition_total_duration_ms(data, nullptr) : 0;
        bool can_start = data && get_expedition_can_try_start
                ? get_expedition_can_try_start(data, nullptr) : false;
        if (game_duration <= 0 && id_text == observed_expedition_task_id) game_duration = observed_expedition_duration_ms;
        const bool is_gift = std::strcmp(kind, "gift") == 0;
        void *utils = data ? *reinterpret_cast<void **>(static_cast<uint8_t *>(data) + 0x48) : nullptr;
        void *scope = data ? *reinterpret_cast<void **>(static_cast<uint8_t *>(data) + 0x98) : nullptr;
        if (utils && !expedition_utils_metadata_logged && object_get_class) {
            expedition_utils_metadata_logged = true;
            log_class_methods("ExpeditionUtilsInstance", object_get_class(utils));
        }
        void *pikmins = return_inventory_manager && get_pikmin_item_collection
                ? get_pikmin_item_collection(return_inventory_manager, nullptr) : nullptr;
        void *picked = utils && pick_fastest_pikmins && pikmins && scope
                ? pick_fastest_pikmins(utils, data, pikmins, scope, nullptr) : nullptr;
        log_dispatch_enumerable_metadata(picked ? picked : pikmins, picked ? "picked" : "pikminCollection");
        int picked_count = count_enumerable_items(picked);
        bool gift_pikmin_unavailable = is_gift && (!picked || picked_count <= 0);
        if (batch_ready && id_text == batch_target && is_gift) {
            if (gift_target && !gift_target_metadata_logged && object_get_class) {
                gift_target_metadata_logged = true;
                log_class_methods("GiftTarget", object_get_class(gift_target));
            }
            LOGI("[GIFT-DIAG] task=%s player=%.7f,%.7f spawn=%.7f,%.7f finish=%.7f,%.7f duration=%" PRId64 " canStart=%d picked=%d",
                 id_text.c_str(), current_latitude, current_longitude, latitude, longitude,
                 finish_latitude, finish_longitude, game_duration, can_start ? 1 : 0, picked_count);
            LOGI("[GIFT-DIAG] task=%s gamePickerCount=%d; eligibility delegated to ExpeditionItemData.Allows",
                 id_text.c_str(), picked_count);
        }
        // Armed mode may start a bounded group of nearby tasks from this one
        // live scan. Batch mode is stricter: the Control Center must name this
        // exact task and prove a fresh five-second arrival gate before any game
        // API is invoked. The native side rechecks the live game location.
        const bool armed_kind_allowed = armed_kind_filter == "all" || armed_kind_filter == kind ||
                (armed_kind_filter == "farm" && (std::strcmp(kind, "fruit") == 0 || std::strcmp(kind, "seed") == 0));
        const bool requested = (armed && armed_kind_allowed) || (batch_ready && id_text == batch_target);
        // Control Center's own arrival gate already requires this same
        // distance column within 4 m (agreeing with the provider) for two
        // consecutive fresh scans before it ever authorises a batch dispatch,
        // so 4 m here does not reject anything the controller would not have
        // rejected itself -- it just makes the native side fail closed on
        // distance too, instead of trusting the controller's word for it.
        // Only trustworthy since current_location() now fails closed on a
        // stale fallback file (see kSystemGpsMaxAgeSeconds); at the old 25 m
        // this same reading was once 111 m wrong for over an hour.
        // Armed mode's own radius: previously a proxy via the game's derived
        // travel duration (<=5 minutes); now a direct GPS check against the
        // same distance reading, at the user's request for a literal radius
        // instead of an indirect time estimate.
        const double allowed_distance = batch ? 4.0 : 200.0;
        const bool armed_capacity_available = armed &&
                armed_dispatches.size() < kArmedMaxInFlight &&
                starts_requested < static_cast<int>(kArmedMaxStartsPerScan) &&
                armed_inflight == armed_dispatches.end();
        const bool batch_capacity_available = batch && dispatch_confirmation_pending_id.empty();
        if ((armed_capacity_available || batch_capacity_available) && requested && data && picked &&
            picked_count > 0 && set_expedition_pikmins &&
            distance >= 0.0 && distance <= allowed_distance) {
            // The picker calls ExpeditionItemData.Allows for every candidate.
            // For ordinary gifts that enforces Restriction.AllowedPikminId;
            // for rare-deco gifts it enforces AllowsRareDecoGift. Never
            // substitute a controller-chosen Pikmin.
            void *ids = picked_pikmin_ids_array(picked);
            if (ids) {
                set_expedition_pikmins(data, ids, nullptr);
                game_duration = original_get_expedition_total_duration_ms
                        ? original_get_expedition_total_duration_ms(data, nullptr) : game_duration;
                can_start = get_expedition_can_try_start
                        ? get_expedition_can_try_start(data, nullptr) : can_start;
                const bool carrying_power_after = get_expedition_has_enough_carrying_power
                        ? get_expedition_has_enough_carrying_power(data, nullptr) : true;
                ++selections_applied;
                LOGI("[DISPATCH-OBSERVE] armed selection task=%s duration=%" PRId64 " canStart=%d picked=%d carryingPower=%d",
                     id_text.c_str(), game_duration, can_start ? 1 : 0, picked_count, carrying_power_after ? 1 : 0);
                // distance is already <= allowed_distance here (checked above),
                // so the radius gate for this dispatch is the distance check,
                // not the game's derived duration -- can_start is the only
                // remaining game-side condition.
                gift_pikmin_unavailable = is_gift && !can_start;
                if (can_start && start_expedition) {
                    // Recorded before the call so a rate-limit hypothesis for
                    // the fault rate (see append_rpc_fault_diagnostics) can be
                    // checked against how soon this attempt followed the last
                    // one, not just whether it faulted.
                    const long long attempt_now = now_ms();
                    pending_expedition_ms_since_previous_attempt = last_expedition_attempt_started_ms > 0
                            ? attempt_now - last_expedition_attempt_started_ms : -1;
                    last_expedition_attempt_started_ms = attempt_now;
                    void *start_result = start_expedition(data, nullptr);
                    const bool can_start_after = get_expedition_can_try_start
                            ? get_expedition_can_try_start(data, nullptr) : false;
                    const int64_t finish_after = get_task_finish_time_ms
                            ? get_task_finish_time_ms(task, nullptr) : 0;
                    start_requested = true;
                    ++starts_requested;
                    if (batch) {
                        dispatch_confirmation_pending_id = id_text;
                        dispatch_confirmation_started_observed_ms = observed_ms;
                    } else {
                        ArmedDispatchInFlight in_flight{};
                        in_flight.task = start_result;
                        in_flight.task_handle = start_result && gchandle_new
                                ? gchandle_new(start_result, false) : 0;
                        in_flight.kind = kind;
                        in_flight.started_ms = now_ms();
                        in_flight.ms_since_previous_attempt = pending_expedition_ms_since_previous_attempt;
                        in_flight.seen_this_scan = true;
                        armed_dispatches.emplace(id_text, std::move(in_flight));
                    }
                    if (batch && start_result && gchandle_new) {
                        pending_expedition_task = start_result;
                        pending_expedition_task_handle = gchandle_new(start_result, false);
                        pending_expedition_task_id = id_text;
                        pending_expedition_task_kind = kind;
                        pending_expedition_task_started_ms = now_ms();
                    }
                    append_dispatch_history("start-requested", kind, id_text, game_duration, picked_count);
                    LOGI("[DISPATCH] start requested task=%s duration=%" PRId64 " picked=%d result=%p canStartAfter=%d finishAfter=%" PRId64,
                         id_text.c_str(), game_duration, picked_count, start_result,
                         can_start_after ? 1 : 0, finish_after);
                } else if (batch_ready && id_text == batch_target && gift_pikmin_unavailable &&
                           dispatch_last_gift_skip_id != id_text) {
                    // The designated ID exists, but the game rejected it (for
                    // example that Pikmin is busy). Never substitute another.
                    append_dispatch_history("gift-pikmin-unavailable", kind, id_text,
                                            game_duration, picked_count);
                    dispatch_last_gift_skip_id = id_text;
                }
            }
        } else if (batch_ready && id_text == batch_target) {
            // The batch's own named target failed the dispatch gate this
            // tick without ever reaching start_expedition() -- Java-side
            // this is invisible and eventually times out as
            // game-refresh-timeout, indistinguishable from an RPC fault
            // without this. Breaks down every sub-condition of the big `if`
            // above so a game_refresh_timeout can be told apart from an
            // actual INVALID_ARGUMENT fault: is it the confirmation lock
            // still held, the team picker returning nothing/zero, or the
            // native distance reading disagreeing with the 4 m batch gate
            // Control Center already thought it satisfied?
            const bool lock_held = !dispatch_confirmation_pending_id.empty();
            LOGI("[DISPATCH-GATE] batch target blocked task=%s lockHeld=%d requested=%d data=%p picked=%p pickedCount=%d distance=%.2fm allowed=%.1fm",
                 id_text.c_str(), lock_held ? 1 : 0, requested ? 1 : 0,
                 data, picked, picked_count, distance, allowed_distance);
            append_dispatch_gate_block(id_text, kind, lock_held, requested, data != nullptr,
                                       picked != nullptr, picked_count, distance, allowed_distance);
            if (gift_pikmin_unavailable && dispatch_last_gift_skip_id != id_text) {
                append_dispatch_history("gift-pikmin-unavailable", kind, id_text,
                                        game_duration, picked_count);
                dispatch_last_gift_skip_id = id_text;
            }
        }
        std::fprintf(file, "%lld\t%s\t%s\t0\t%" PRId64 "\t%.7f\t%.7f\tpending-travel-estimate\t%.1f\t%" PRId64 "\t%d\t%d\n", observed_ms,
                     kind, id_text.c_str(),
                     expiration_ms, latitude, longitude, distance, game_duration, can_start ? 1 : 0, picked_count);
        ++candidates;
    }
    // Tick heartbeat, batch mode only: proves this observation tick actually
    // ran (vs. the whole native tick loop having silently stopped, e.g. the
    // same underlying cause already found for "掃描" returning 0
    // candidates) and separately whether the batch's named target was even
    // present in the raw list this tick (vs. present but blocked -- see the
    // "batch target blocked" log above). A long stretch of no new lines
    // here during a game-refresh-timeout means the tick loop itself
    // stalled; lines present but batchTargetSeen=0 means the target
    // transiently vanished from what the game handed back.
    if (batch && !batch_target.empty()) {
        append_dispatch_tick_trace(batch_target, batch_target_seen_raw, count, candidates);
    }
    // GetPikminTaskList is the live inventory projection. Once a task that was
    // just submitted is absent from a later complete projection, record that
    // separately from the request rather than pretending the async call alone
    // was confirmation.
    if (confirmation_due && !dispatch_confirmation_pending_id.empty() && !pending_confirmation_seen) {
        append_dispatch_history("inventory-confirmed-absent", "unknown",
                                dispatch_confirmation_pending_id, 0, 0);
        dispatch_confirmation_pending_id.clear();
        dispatch_confirmation_started_observed_ms = 0;
    }
    // A complete task-list projection that no longer contains an armed call
    // is the same inventory confirmation used by batch mode, but evaluated
    // independently for every concurrent armed start.
    for (auto it = armed_dispatches.begin(); it != armed_dispatches.end();) {
        if (!it->second.seen_this_scan) {
            append_dispatch_history("inventory-confirmed-absent", it->second.kind.c_str(),
                                    it->first, 0, 0);
            it = armed_dispatches.erase(it);
        } else {
            ++it;
        }
    }
    std::fclose(file);
    chmod(dispatch_candidates_path, 0644);
    FILE *status = std::fopen(dispatch_status_path, "w");
    if (!status) return;
    std::fprintf(status, "%lld\t%s\t%d\t%s\n", observed_ms,
                 armed ? "armed" : (batch ? "batch" : "observed"), candidates,
                 start_requested ? "start-requested" :
                 (selections_applied > 0 ? "selection-applied" :
                  (batch && !batch_ready ? "awaiting-arrival-gate" : "no-dispatch")));
    std::fclose(status);
    chmod(dispatch_status_path, 0644);
    FILE *diagnostics = std::fopen(dispatch_diagnostics_path, "w");
    if (!diagnostics) return;
    std::fprintf(diagnostics, "observed=%lld\texpeditions=%d\tfinish_nonzero=%d\tcandidates=%d\n",
                 observed_ms, expedition_tasks, finished_expedition_tasks, candidates);
    std::fprintf(diagnostics, "targets\tseed=%d\tfruit=%d\tgift=%d\tbloomed_poi=%d\tother=%d\n",
                 direct_seed_targets, direct_fruit_targets, gift_targets, bloomed_poi_targets, other_targets);
    for (const auto &entry : target_cases) {
        const auto unfinished = unfinished_target_cases.find(entry.first);
        std::fprintf(diagnostics, "target_case=%d\ttotal=%d\tfinish_zero=%d\n", entry.first,
                     entry.second, unfinished == unfinished_target_cases.end() ? 0 : unfinished->second);
    }
    std::fclose(diagnostics);
    chmod(dispatch_diagnostics_path, 0644);
}

void *hooked_complete_pikmin_task(void *self, void *task_id, bool discard_postcard, void *method_info) {
    append_return_trace(discard_postcard ? "manual-discard-postcard" : "manual-keep-postcard", self, task_id);
    return original_complete_pikmin_task
            ? original_complete_pikmin_task(self, task_id, discard_postcard, method_info) : nullptr;
}

// The preparer receives the game's own collection of returned task ids before
// a reward is claimed.  This trace is deliberately passive: it establishes a
// version-safe source for background work without reading guessed HashSet
// layouts or emitting any RPCs.
void *hooked_prepare_pikmin_tasks(void *self, void *task_ids, void *method_info) {
    LOGI("[RETURN-DIAG] prepare-pending self=%p taskIds=%p", self, task_ids);
    return original_prepare_pikmin_tasks
            ? original_prepare_pikmin_tasks(self, task_ids, method_info) : nullptr;
}

// This method is the game's own readiness gate.  It lets us capture only task
// ids that the client itself considers completable, rather than probing every
// expedition or inferring state from UI objects.
bool hooked_should_prepare_completion(void *self, void *task, void *method_info) {
    const bool ready = original_should_prepare_completion
            && original_should_prepare_completion(self, task, method_info);
    if (!ready || !task || !get_inventory_item_id) return ready;
    void *task_id = get_inventory_item_id(task, nullptr);
    append_return_trace("ready-task", self, task_id);
    return ready;
}

// The expedition page reads its cards from this inventory list.  Keep this
// diagnostic read-only until both its current layout and the game's readiness
// predicate have been observed on-device.
void *hooked_get_pikmin_task_list(void *self, void *method_info) {
    void *result = original_get_pikmin_task_list
            ? original_get_pikmin_task_list(self, method_info) : nullptr;
    const long long now = now_ms();
    if (!result || !get_inventory_item_id || !get_pikmin_task_proto || !get_task_finish_time_ms ||
        now - last_task_list_trace_ms < 5000) return result;
    last_task_list_trace_ms = now;

    // System.Collections.Generic.List<T>: _items at +0x10, _size at +0x18;
    // managed arrays store their first element at +0x20 on arm64 IL2CPP.
    auto *bytes = static_cast<uint8_t *>(result);
    void *items = *reinterpret_cast<void **>(bytes + 0x10);
    const int count = *reinterpret_cast<int *>(bytes + 0x18);
    if (!items || count < 0 || count > kMaxPikminTaskListCount) {
        LOGE("[RETURN-DIAG] task-list invalid self=%p list=%p count=%d", self, result, count);
        return result;
    }
    LOGI("[RETURN-DIAG] task-list self=%p count=%d", self, count);
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!task) continue;
        void *id = get_inventory_item_id(task, nullptr);
        void *proto = get_pikmin_task_proto(task, nullptr);
        const int64_t finish_ms = proto ? get_task_finish_time_ms(proto, nullptr) : 0;
        LOGI("[RETURN-DIAG] task-list-item id=%s finishMs=%" PRId64 " due=%d",
             utf8_string(id).c_str(), finish_ms, finish_ms > 0 && finish_ms <= now ? 1 : 0);
    }
    return result;
}

// This constructor is a one-time Zenject injection point, not a UI rendering
// path.  It provides the exact live manager instances used by the game.
void hooked_pikmin_task_preparer_constructor(void *self, void *inventory, void *rpc,
                                             void *server_clock, void *method_info) {
    if (original_pikmin_task_preparer_constructor) {
        original_pikmin_task_preparer_constructor(self, inventory, rpc, server_clock, method_info);
    }
    return_preparer = self;
    return_inventory_manager = inventory;
    LOGI("[RETURN-DIAG] preparer captured self=%p inventory=%p rpc=%p clock=%p",
         self, inventory, rpc, server_clock);
}

// Capture the game's own action manager. No return RPC is invoked here.
void hooked_pikmin_task_action_manager_constructor(void *self, void *method_info) {
    if (original_pikmin_task_action_manager_constructor) {
        original_pikmin_task_action_manager_constructor(self, method_info);
    }
    return_action_manager = self;
    LOGI("[RETURN-DIAG] action manager captured self=%p", self);
}

// The store owns every live expedition data item, including entries whose UI
// page has never been opened.  Capturing it lets the background scanner use
// the game's own data object for each inventory task.
void hooked_expedition_data_store_constructor(void *self, void *method_info) {
    if (original_expedition_data_store_constructor) {
        original_expedition_data_store_constructor(self, method_info);
    }
    expedition_data_store = self;
    LOGI("[DISPATCH-OBSERVE] expedition data store captured self=%p", self);
}

// The picker evaluates this after selecting its fastest valid Pikmin.  By
// observing it we can use the game's own round-trip calculation, rather than
// converting map distance to a guessed travel time.  This hook is read-only.
int64_t hooked_get_expedition_total_duration_ms(void *self, void *method_info) {
    const int64_t duration = original_get_expedition_total_duration_ms
            ? original_get_expedition_total_duration_ms(self, method_info) : 0;
    const long long current = now_ms();
    if (self && duration > 0 && current - last_expedition_duration_observed_ms >= 1000) {
        last_expedition_duration_observed_ms = current;
        void *key = get_expedition_item_key ? get_expedition_item_key(self, nullptr) : nullptr;
        observed_expedition_task_id = utf8_string(key);
        observed_expedition_duration_ms = duration;
        LOGI("[DISPATCH-OBSERVE] picker task=%s totalDurationMs=%" PRId64,
             observed_expedition_task_id.c_str(), duration);
    }
    return duration;
}

// Runs on the game's main update thread.  It asks the game's own readiness
// predicate about each live inventory task and only writes diagnostics.
void dry_run_return_tasks() {
    const long long now = now_ms();
    if (now - last_return_dry_run_ms < 5000) return;
    last_return_dry_run_ms = now;
    if (!return_preparer || !return_inventory_manager || !original_get_pikmin_task_list ||
        !original_should_prepare_completion || !get_inventory_item_id) {
        LOGI("[RETURN-DIAG] dry-run waiting preparer=%p inventory=%p taskList=%p shouldPrepare=%p getId=%p",
             return_preparer, return_inventory_manager,
             reinterpret_cast<void *>(original_get_pikmin_task_list),
             reinterpret_cast<void *>(original_should_prepare_completion),
             reinterpret_cast<void *>(get_inventory_item_id));
        return;
    }
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) {
        LOGI("[RETURN-DIAG] dry-run task-list null inventory=%p", return_inventory_manager);
        return;
    }
    write_dispatch_candidates(list, now);
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > kMaxPikminTaskListCount) {
        LOGI("[RETURN-DIAG] dry-run invalid list=%p items=%p count=%d", list, items, count);
        return;
    }
    int ready_count{};
    int due_count{};
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!task) continue;
        void *task_id = get_inventory_item_id(task, nullptr);
        if (original_should_prepare_completion(return_preparer, task, nullptr)) {
            ++ready_count;
            append_return_trace("dry-run-ready", return_preparer, task_id);
        }
        void *proto = get_pikmin_task_proto ? get_pikmin_task_proto(task, nullptr) : nullptr;
        const int64_t finish_ms = proto && get_task_finish_time_ms
                ? get_task_finish_time_ms(proto, nullptr) : 0;
        if (finish_ms > 0 && finish_ms <= now) {
            ++due_count;
            append_return_trace("dry-run-finish-due", return_preparer, task_id);
            log_task_variant_metadata(proto);
        }
    }
    LOGI("[RETURN-DIAG] dry-run tasks=%d preparerReady=%d finishDue=%d",
         count, ready_count, due_count);
}

void *find_class(const char *namespc, const char *name) {
    if (!game_domain || !domain_get_assemblies || !assembly_get_image || !class_from_name) return nullptr;
    size_t count{};
    const void **assemblies = domain_get_assemblies(game_domain, &count);
    for (size_t index = 0; assemblies && index < count; ++index) {
        void *klass = class_from_name(assembly_get_image(assemblies[index]), namespc, name);
        if (klass) return klass;
    }
    return nullptr;
}

// Metadata-only probe.  It never invokes an unknown method or touches a task;
// it simply records the public method names exposed by this exact game build so
// reward fields can be decoded without relying on guessed object offsets.
void log_class_methods(const char *label, void *proto_class) {
    if (!proto_class || !class_get_methods || !method_get_name || !method_get_param_count) return;
    void *iterator = nullptr;
    int logged{};
    while (void *method = class_get_methods(proto_class, &iterator)) {
        const char *name = method_get_name(method);
        if (!name) continue;
        LOGI("[RETURN-META] %s method=%s params=%u", label, name,
             method_get_param_count(method));
        if (++logged >= 160) {
            LOGE("[RETURN-META] method enumeration capped at 160 entries");
            break;
        }
    }
    LOGI("[RETURN-META] %s metadata methods=%d", label, logged);
}

void log_task_proto_methods(void *proto_class) {
    log_class_methods("PikminTaskProto", proto_class);
}

void log_task_variant_metadata(void *proto) {
    if (!proto || !object_get_class || !class_get_name) return;
    const GetTaskVariant getters[] = {get_task_carry, get_task_expedition, get_task_gift, get_task_poi_challenge};
    const char *labels[] = {"Carry", "Expedition", "Gift", "PoiChallenge"};
    for (int index = 0; index < 4; ++index) {
        if (!getters[index] || return_reward_variant_metadata_logged[index]) continue;
        void *variant = getters[index](proto, nullptr);
        if (!variant) continue;
        void *klass = object_get_class(variant);
        const char *name = klass ? class_get_name(klass) : "unknown";
        LOGI("[RETURN-META] active task variant=%s class=%s", labels[index], name ? name : "unknown");
        log_class_methods(labels[index], klass);
        if (index == 1) {
            const GetTaskVariant getters[] = {get_expedition_seed, get_expedition_fruit, get_expedition_gift,
                    get_expedition_bloomed_poi, get_expedition_postcard, get_expedition_postcard_with_items};
            const char *target_labels[] = {"Seed", "Fruit", "ExpeditionGift", "BloomedPoi", "Postcard", "PostcardWithItems"};
            for (int target = 0; target < 6; ++target) {
                if (!getters[target] || return_expedition_target_metadata_logged[target]) continue;
                void *value = getters[target](variant, nullptr);
                if (!value) continue;
                void *value_class = object_get_class(value);
                const char *value_name = value_class ? class_get_name(value_class) : "unknown";
                LOGI("[RETURN-META] active expedition target=%s class=%s", target_labels[target],
                     value_name ? value_name : "unknown");
                log_class_methods(target_labels[target], value_class);
                if (target == 3 && get_bloomed_poi_reward_v2 && !return_bloomed_poi_reward_metadata_logged) {
                    void *reward = get_bloomed_poi_reward_v2(value, nullptr);
                    if (reward) {
                        void *reward_class = object_get_class(reward);
                        const char *reward_name = reward_class ? class_get_name(reward_class) : "unknown";
                        LOGI("[RETURN-META] BloomedPoi RewardV2 class=%s", reward_name ? reward_name : "unknown");
                        log_class_methods("BloomedPoiRewardV2", reward_class);
                        return_bloomed_poi_reward_metadata_logged = true;
                        if (get_poi_reward_fruit && !return_bloomed_poi_fruit_metadata_logged) {
                            void *fruit = get_poi_reward_fruit(reward, nullptr);
                            if (fruit) {
                                void *fruit_class = object_get_class(fruit);
                                const char *fruit_name = fruit_class ? class_get_name(fruit_class) : "unknown";
                                LOGI("[RETURN-META] BloomedPoi Fruit class=%s", fruit_name ? fruit_name : "unknown");
                                log_class_methods("BloomedPoiFruit", fruit_class);
                                return_bloomed_poi_fruit_metadata_logged = true;
                                auto *repeated = static_cast<uint8_t *>(fruit);
                                void *items = *reinterpret_cast<void **>(repeated + 0x10);
                                const int count = *reinterpret_cast<int *>(repeated + 0x18);
                                if (!return_bloomed_poi_fruit_entry_metadata_logged && items && count > 0 && count <= 32) {
                                    void *entry = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20);
                                    void *entry_class = entry ? object_get_class(entry) : nullptr;
                                    const char *entry_name = entry_class ? class_get_name(entry_class) : "unknown";
                                    LOGI("[RETURN-META] BloomedPoi Fruit entry class=%s count=%d",
                                         entry_name ? entry_name : "unknown", count);
                                    log_class_methods("BloomedPoiFruitEntry", entry_class);
                                    return_bloomed_poi_fruit_entry_metadata_logged = true;
                                }
                            }
                        }
                    }
                }
                return_expedition_target_metadata_logged[target] = true;
            }
        }
        return_reward_variant_metadata_logged[index] = true;
        return;
    }
}

void install_return_diagnostic_hook() {
    if (!class_get_method_from_name) {
        LOGE("[RETURN-DIAG] il2cpp_class_get_method_from_name unavailable");
        return;
    }
    void *klass = find_class("Niantic.Ichigo.Game.PikminTasks", "PikminTaskActionManager");
    if (!klass) {
        LOGE("[RETURN-DIAG] PikminTaskActionManager class not found");
        return;
    }
    void *action_ctor = class_get_method_from_name(klass, ".ctor", 0);
    void *action_ctor_entry = action_ctor ? *reinterpret_cast<void **>(action_ctor) : nullptr;
    if (!action_ctor_entry) {
        LOGE("[RETURN-DIAG] PikminTaskActionManager constructor not found");
        return;
    }
    A64HookFunction(action_ctor_entry, reinterpret_cast<void *>(hooked_pikmin_task_action_manager_constructor),
                    reinterpret_cast<void **>(&original_pikmin_task_action_manager_constructor));
    LOGI("[RETURN-DIAG] action-manager constructor hook installed method=%p entry=%p",
         action_ctor, action_ctor_entry);
    // This is the public overload that contains both the task id and postcard policy.
    void *method = class_get_method_from_name(klass, "CompletePikminTaskAsync", 2);
    if (!method) {
        LOGE("[RETURN-DIAG] CompletePikminTaskAsync(taskId, discardPostcard) not found");
        return;
    }
    void *entry = *reinterpret_cast<void **>(method);
    if (!entry) {
        LOGE("[RETURN-DIAG] CompletePikminTaskAsync has no native entry point");
        return;
    }
    A64HookFunction(entry, reinterpret_cast<void *>(hooked_complete_pikmin_task),
                    reinterpret_cast<void **>(&original_complete_pikmin_task));
    LOGI("[RETURN-DIAG] hook installed method=%p entry=%p", method, entry);

    void *preparer = find_class("Niantic.Ichigo.Game.PikminTasks", "PikminTaskCompletionPreparer");
    void *preparer_ctor = preparer ? class_get_method_from_name(preparer, ".ctor", 3) : nullptr;
    void *preparer_ctor_entry = preparer_ctor ? *reinterpret_cast<void **>(preparer_ctor) : nullptr;
    if (!preparer_ctor_entry) {
        LOGE("[RETURN-DIAG] PikminTaskCompletionPreparer constructor not found");
        return;
    }
    A64HookFunction(preparer_ctor_entry, reinterpret_cast<void *>(hooked_pikmin_task_preparer_constructor),
                    reinterpret_cast<void **>(&original_pikmin_task_preparer_constructor));
    LOGI("[RETURN-DIAG] preparer constructor hook installed method=%p entry=%p",
         preparer_ctor, preparer_ctor_entry);
    void *prepare_method = preparer
            ? class_get_method_from_name(preparer, "PrepareCompletionAsync", 1) : nullptr;
    void *prepare_entry = prepare_method ? *reinterpret_cast<void **>(prepare_method) : nullptr;
    if (!prepare_entry) {
        LOGE("[RETURN-DIAG] PrepareCompletionAsync(taskIds) not found");
        return;
    }
    A64HookFunction(prepare_entry, reinterpret_cast<void *>(hooked_prepare_pikmin_tasks),
                    reinterpret_cast<void **>(&original_prepare_pikmin_tasks));
    LOGI("[RETURN-DIAG] preparer hook installed method=%p entry=%p", prepare_method, prepare_entry);

    void *ready_method = class_get_method_from_name(preparer, "ShouldPrepareCompletion", 1);
    void *ready_entry = ready_method ? *reinterpret_cast<void **>(ready_method) : nullptr;
    void *task_class = find_class("Niantic.Ichigo.Inventory", "PikminTaskInventoryItem");
    void *id_method = task_class
            ? class_get_method_from_name(task_class, "get_Id", 0) : nullptr;
    void *id_entry = id_method ? *reinterpret_cast<void **>(id_method) : nullptr;
    if (!ready_entry || !id_entry) {
        LOGE("[RETURN-DIAG] readiness or task-id method not found");
        return;
    }
    get_inventory_item_id = reinterpret_cast<GetInventoryItemId>(id_entry);
    A64HookFunction(ready_entry, reinterpret_cast<void *>(hooked_should_prepare_completion),
                    reinterpret_cast<void **>(&original_should_prepare_completion));
    LOGI("[RETURN-DIAG] readiness hook installed method=%p entry=%p", ready_method, ready_entry);

    void *inventory = find_class("Niantic.Ichigo.Inventory", "InventoryManager");
    void *list_method = inventory ? class_get_method_from_name(inventory, "GetPikminTaskList", 0) : nullptr;
    void *list_entry = list_method ? *reinterpret_cast<void **>(list_method) : nullptr;
    void *proto_method = task_class ? class_get_method_from_name(task_class, "get_Proto", 0) : nullptr;
    void *proto_entry = proto_method ? *reinterpret_cast<void **>(proto_method) : nullptr;
    void *proto_class = find_class("Ichigo.Proto", "PikminTaskProto");
    log_task_proto_methods(proto_class);
    void *finish_method = proto_class ? class_get_method_from_name(proto_class, "get_FinishTimeMs", 0) : nullptr;
    void *finish_entry = finish_method ? *reinterpret_cast<void **>(finish_method) : nullptr;
    if (!list_entry || !proto_entry || !finish_entry) {
        LOGE("[RETURN-DIAG] task-list or task-time method not found");
        return;
    }
    get_pikmin_task_proto = reinterpret_cast<GetPikminTaskProto>(proto_entry);
    get_task_finish_time_ms = reinterpret_cast<GetTaskFinishTimeMs>(finish_entry);
    void *finish_location_method = proto_class
            ? class_get_method_from_name(proto_class, "get_FinishLocation", 0) : nullptr;
    get_task_finish_location = finish_location_method
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(finish_location_method)) : nullptr;
    void *task_pikmin_id_method = proto_class
            ? class_get_method_from_name(proto_class, "get_PikminId", 0) : nullptr;
    get_task_pikmin_id = task_pikmin_id_method
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(task_pikmin_id_method)) : nullptr;
    void *expedition_item_class = find_class("Niantic.Ichigo.Game.Expedition.Data", "ExpeditionItemData");
    void *expedition_key_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "get_Key", 0) : nullptr;
    void *expedition_duration_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "get_TotalDurationMs", 0) : nullptr;
    void *expedition_duration_entry = expedition_duration_method
            ? *reinterpret_cast<void **>(expedition_duration_method) : nullptr;
    get_expedition_item_key = expedition_key_method
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(expedition_key_method)) : nullptr;
    if (expedition_duration_entry) {
        A64HookFunction(expedition_duration_entry,
                        reinterpret_cast<void *>(hooked_get_expedition_total_duration_ms),
                        reinterpret_cast<void **>(&original_get_expedition_total_duration_ms));
        LOGI("[DISPATCH-OBSERVE] duration hook installed method=%p entry=%p",
             expedition_duration_method, expedition_duration_entry);
    } else {
        LOGE("[DISPATCH-OBSERVE] ExpeditionItemData.TotalDurationMs not found");
    }
    void *expedition_store_class = find_class("Niantic.Ichigo.Game.Expedition.Data", "ExpeditionDataStore");
    void *expedition_store_ctor = expedition_store_class
            ? class_get_method_from_name(expedition_store_class, ".ctor", 0) : nullptr;
    void *expedition_store_ctor_entry = expedition_store_ctor
            ? *reinterpret_cast<void **>(expedition_store_ctor) : nullptr;
    void *by_index_method = expedition_store_class
            ? class_get_method_from_name(expedition_store_class, "ByIndex", 2) : nullptr;
    get_expedition_data_by_index = by_index_method
            ? reinterpret_cast<GetExpeditionDataByIndex>(*reinterpret_cast<void **>(by_index_method)) : nullptr;
    void *can_start_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "get_CanTryStart", 0) : nullptr;
    get_expedition_can_try_start = can_start_method
            ? reinterpret_cast<GetExpeditionBool>(*reinterpret_cast<void **>(can_start_method)) : nullptr;
    void *carrying_power_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "get_HasEnoughCarryingPower", 0) : nullptr;
    get_expedition_has_enough_carrying_power = carrying_power_method
            ? reinterpret_cast<GetExpeditionBool>(*reinterpret_cast<void **>(carrying_power_method)) : nullptr;
    void *invalidate_cached_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "InvalidateAllCachedValues", 0) : nullptr;
    invalidate_expedition_cached_values = invalidate_cached_method
            ? reinterpret_cast<SimpleMethod>(*reinterpret_cast<void **>(invalidate_cached_method)) : nullptr;
    LOGI("[DISPATCH-OBSERVE] carrying power method=%p invalidateCached method=%p",
         carrying_power_method, invalidate_cached_method);
    void *set_pikmins_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "SetPikmins", 1) : nullptr;
    set_expedition_pikmins = set_pikmins_method
            ? reinterpret_cast<SetExpeditionPikmins>(*reinterpret_cast<void **>(set_pikmins_method)) : nullptr;
    void *start_expedition_method = expedition_item_class
            ? class_get_method_from_name(expedition_item_class, "StartExpeditionAsync", 0) : nullptr;
    start_expedition = start_expedition_method
            ? reinterpret_cast<StartExpedition>(*reinterpret_cast<void **>(start_expedition_method)) : nullptr;
    void *pikmin_collection_method = inventory
            ? class_get_method_from_name(inventory, "get_PikminItemCollection", 0) : nullptr;
    get_pikmin_item_collection = pikmin_collection_method
            ? reinterpret_cast<GetEnumerable>(*reinterpret_cast<void **>(pikmin_collection_method)) : nullptr;
    void *utils_class = find_class("Niantic.Ichigo.Game.Expedition", "ExpeditionUtils");
    void *pick_fastest_method = utils_class ? class_get_method_from_name(utils_class,
            "PickFastestUpToItemLimitsReservingForTroop", 3) : nullptr;
    pick_fastest_pikmins = pick_fastest_method
            ? reinterpret_cast<PickFastestPikmins>(*reinterpret_cast<void **>(pick_fastest_method)) : nullptr;
    if (expedition_store_ctor_entry) {
        A64HookFunction(expedition_store_ctor_entry,
                        reinterpret_cast<void *>(hooked_expedition_data_store_constructor),
                        reinterpret_cast<void **>(&original_expedition_data_store_constructor));
        LOGI("[DISPATCH-OBSERVE] store hook installed ctor=%p byIndex=%p canTryStart=%p pickFastest=%p setPikmins=%p start=%p",
             expedition_store_ctor_entry, by_index_method, can_start_method, pick_fastest_method,
             set_pikmins_method, start_expedition_method);
    } else {
        LOGE("[DISPATCH-OBSERVE] ExpeditionDataStore constructor not found");
    }
    void *task_case = proto_class ? class_get_method_from_name(proto_class, "get_TaskCase", 0) : nullptr;
    get_task_case = task_case ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(task_case)) : nullptr;
    void *carry = proto_class ? class_get_method_from_name(proto_class, "get_Carry", 0) : nullptr;
    void *expedition = proto_class ? class_get_method_from_name(proto_class, "get_Expedition", 0) : nullptr;
    void *gift = proto_class ? class_get_method_from_name(proto_class, "get_Gift", 0) : nullptr;
    void *poi_challenge = proto_class ? class_get_method_from_name(proto_class, "get_PoiChallenge", 0) : nullptr;
    get_task_carry = carry ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(carry)) : nullptr;
    get_task_expedition = expedition ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(expedition)) : nullptr;
    get_task_gift = gift ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(gift)) : nullptr;
    get_task_poi_challenge = poi_challenge ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(poi_challenge)) : nullptr;
    void *carry_class = find_class("Ichigo.Proto", "CarryTaskProto");
    void *carry_resource = carry_class ? class_get_method_from_name(carry_class, "get_Resource", 0) : nullptr;
    void *carry_seed = carry_class ? class_get_method_from_name(carry_class, "get_PikminSeed", 0) : nullptr;
    get_carry_resource = carry_resource ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(carry_resource)) : nullptr;
    get_carry_pikmin_seed = carry_seed ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(carry_seed)) : nullptr;
    void *seed_class = find_class("Ichigo.Proto", "PikminSeedProto");
    void *seed_type = seed_class ? class_get_method_from_name(seed_class, "get_SeedType", 0) : nullptr;
    void *seed_pikmin = seed_class ? class_get_method_from_name(seed_class, "get_Pikmin", 0) : nullptr;
    get_seed_type = seed_type ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(seed_type)) : nullptr;
    get_seed_pikmin = seed_pikmin ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(seed_pikmin)) : nullptr;
    void *expedition_class = find_class("Ichigo.Proto", "ExpeditionTaskProto");
    void *expedition_target_case = expedition_class
            ? class_get_method_from_name(expedition_class, "get_TargetCase", 0) : nullptr;
    get_expedition_target_case = expedition_target_case
            ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(expedition_target_case)) : nullptr;
    void *expedition_expiration = expedition_class
            ? class_get_method_from_name(expedition_class, "get_ExpirationTimeMs", 0) : nullptr;
    void *expedition_spawn_point = expedition_class
            ? class_get_method_from_name(expedition_class, "get_SpawnPoint", 0) : nullptr;
    get_expedition_expiration_time_ms = expedition_expiration
            ? reinterpret_cast<GetTaskLong>(*reinterpret_cast<void **>(expedition_expiration)) : nullptr;
    get_expedition_spawn_point = expedition_spawn_point
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(expedition_spawn_point)) : nullptr;
    void *point_class = find_class("Ichigo.Proto", "PointProto");
    void *point_lat = point_class ? class_get_method_from_name(point_class, "get_LatDegrees", 0) : nullptr;
    void *point_lng = point_class ? class_get_method_from_name(point_class, "get_LngDegrees", 0) : nullptr;
    get_point_lat_degrees = point_lat ? reinterpret_cast<GetTaskDouble>(*reinterpret_cast<void **>(point_lat)) : nullptr;
    get_point_lng_degrees = point_lng ? reinterpret_cast<GetTaskDouble>(*reinterpret_cast<void **>(point_lng)) : nullptr;
    const char *target_getters[] = {"get_Seed", "get_Fruit", "get_Gift", "get_BloomedPoi", "get_Postcard", "get_PostcardWithItems"};
    GetTaskVariant *target_functions[] = {&get_expedition_seed, &get_expedition_fruit, &get_expedition_gift,
            &get_expedition_bloomed_poi, &get_expedition_postcard, &get_expedition_postcard_with_items};
    for (int index = 0; index < 6; ++index) {
        void *method = expedition_class ? class_get_method_from_name(expedition_class, target_getters[index], 0) : nullptr;
        *target_functions[index] = method ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(method)) : nullptr;
    }
    void *pikmin_type_class = find_class("Ichigo.Proto", "PikminTypeProto");
    void *pikmin_type = pikmin_type_class ? class_get_method_from_name(pikmin_type_class, "get_Type", 0) : nullptr;
    void *pikmin_category = pikmin_type_class ? class_get_method_from_name(pikmin_type_class, "get_CategoryId", 0) : nullptr;
    void *pikmin_full_asset = pikmin_type_class ? class_get_method_from_name(pikmin_type_class, "get_FullAssetId", 0) : nullptr;
    get_pikmin_type = pikmin_type ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(pikmin_type)) : nullptr;
    get_pikmin_category = pikmin_category ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(pikmin_category)) : nullptr;
    get_pikmin_full_asset = pikmin_full_asset ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(pikmin_full_asset)) : nullptr;
    void *gift_class = find_class("Ichigo.Proto", "PikminGiftProto");
    void *gift_item_case = gift_class ? class_get_method_from_name(gift_class, "get_ItemCase", 0) : nullptr;
    void *gift_deco = gift_class ? class_get_method_from_name(gift_class, "get_Deco", 0) : nullptr;
    void *gift_rare_deco = gift_class ? class_get_method_from_name(gift_class, "get_RareDeco", 0) : nullptr;
    get_gift_item_case = gift_item_case ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(gift_item_case)) : nullptr;
    get_gift_deco = gift_deco ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(gift_deco)) : nullptr;
    get_gift_rare_deco = gift_rare_deco ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(gift_rare_deco)) : nullptr;
    void *deco_class = find_class("Ichigo.Proto", "PikminDecorationProto");
    void *deco_pikmin_type = deco_class ? class_get_method_from_name(deco_class, "get_PikminType", 0) : nullptr;
    void *deco_box_type = deco_class ? class_get_method_from_name(deco_class, "get_BoxType", 0) : nullptr;
    get_deco_pikmin_type = deco_pikmin_type ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(deco_pikmin_type)) : nullptr;
    get_deco_box_type = deco_box_type ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(deco_box_type)) : nullptr;
    void *rare_deco_class = find_class("Ichigo.Proto", "PikminRareDecorationProto");
    void *rare_deco_category = rare_deco_class ? class_get_method_from_name(rare_deco_class, "get_CategoryId", 0) : nullptr;
    get_rare_deco_category = rare_deco_category ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(rare_deco_category)) : nullptr;
    void *resource_class = find_class("Ichigo.Proto", "ResourceProto");
    void *resource_type = resource_class ? class_get_method_from_name(resource_class, "get_Type", 0) : nullptr;
    void *resource_pikmins = resource_class ? class_get_method_from_name(resource_class, "get_NumPikmins", 0) : nullptr;
    void *resource_flower = resource_class ? class_get_method_from_name(resource_class, "get_FlowerKind", 0) : nullptr;
    void *resource_honey_flower = resource_class ? class_get_method_from_name(resource_class, "get_HoneyBallFlowerKind", 0) : nullptr;
    void *resource_weight = resource_class ? class_get_method_from_name(resource_class, "get_WeightGrams", 0) : nullptr;
    void *resource_honey_balls = resource_class ? class_get_method_from_name(resource_class, "get_HoneyBall", 0) : nullptr;
    get_resource_type = resource_type ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(resource_type)) : nullptr;
    get_resource_num_pikmins = resource_pikmins ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(resource_pikmins)) : nullptr;
    get_resource_flower_kind = resource_flower ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(resource_flower)) : nullptr;
    get_resource_honey_flower_kind = resource_honey_flower ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(resource_honey_flower)) : nullptr;
    get_resource_weight_grams = resource_weight ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(resource_weight)) : nullptr;
    get_resource_honey_balls = resource_honey_balls ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(resource_honey_balls)) : nullptr;
    void *honey_class = find_class("Ichigo.Proto", "HoneyBallProto");
    void *honey_num_balls = honey_class ? class_get_method_from_name(honey_class, "get_NumBalls", 0) : nullptr;
    void *honey_type = honey_class ? class_get_method_from_name(honey_class, "get_HoneyType", 0) : nullptr;
    void *honey_flower_kind = honey_class ? class_get_method_from_name(honey_class, "get_HoneyFlowerKind", 0) : nullptr;
    void *honey_flower_text = honey_class ? class_get_method_from_name(honey_class, "get_FlowerKind", 0) : nullptr;
    get_honey_num_balls = honey_num_balls ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(honey_num_balls)) : nullptr;
    get_honey_type = honey_type ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(honey_type)) : nullptr;
    get_honey_flower_kind = honey_flower_kind ? reinterpret_cast<GetTaskInt>(*reinterpret_cast<void **>(honey_flower_kind)) : nullptr;
    get_honey_flower_kind_text = honey_flower_text ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(honey_flower_text)) : nullptr;
    void *bloomed_poi_class = find_class("Ichigo.Proto", "PoiFlowerBloomedRewardProto");
    void *reward_v2 = bloomed_poi_class
            ? class_get_method_from_name(bloomed_poi_class, "get_RewardV2", 0) : nullptr;
    get_bloomed_poi_reward_v2 = reward_v2
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(reward_v2)) : nullptr;
    void *poi_reward_class = find_class("Ichigo.Proto", "PoiRewardProto");
    void *poi_reward_fruit = poi_reward_class
            ? class_get_method_from_name(poi_reward_class, "get_Fruit", 0) : nullptr;
    get_poi_reward_fruit = poi_reward_fruit
            ? reinterpret_cast<GetTaskVariant>(*reinterpret_cast<void **>(poi_reward_fruit)) : nullptr;
    A64HookFunction(list_entry, reinterpret_cast<void *>(hooked_get_pikmin_task_list),
                    reinterpret_cast<void **>(&original_get_pikmin_task_list));
    LOGI("[RETURN-DIAG] task-list hook installed method=%p entry=%p", list_method, list_entry);
}

// This must run from a post-login game callback.  On v152 the library is
// mapped long before its domain is usable, and calling domain_get from the
// detached Zygisk worker can crash inside libil2cpp itself.
void initialize_runtime_metadata() {
    if (runtime_metadata_ready || !domain_get) return;
    void *domain = domain_get();
    if (!domain) {
        LOGW("[NECTAR] post-login IL2CPP domain unavailable; metadata deferred");
        return;
    }
    game_domain = domain;
    runtime_metadata_ready = true;
    install_return_diagnostic_hook();
    LOGI("[NECTAR] post-login IL2CPP metadata installed");
}

long long now_ms() {
    timespec value{};
    clock_gettime(CLOCK_REALTIME, &value);
    return static_cast<long long>(value.tv_sec) * 1000LL + value.tv_nsec / 1000000LL;
}

std::string read_string(void *value) {
    if (!value) return {};
    const int length = *reinterpret_cast<int *>(static_cast<uint8_t *>(value) + 0x10);
    if (length <= 0 || length > 512) return {};
    auto *characters = reinterpret_cast<uint16_t *>(static_cast<uint8_t *>(value) + 0x14);
    std::string result;
    result.reserve(static_cast<size_t>(length));
    for (int index = 0; index < length; ++index) {
        const uint16_t character = characters[index];
        result.push_back(character < 0x80 ? static_cast<char>(character) : '?');
    }
    return result;
}

// Best-effort, read-only lookup of a faulted Task's exception, entirely
// through the same dynamic class/method resolution already used safely
// elsewhere in this file (find_class + class_get_method_from_name +
// object_get_virtual_method for polymorphic dispatch) -- never a hardcoded
// RVA, so a method that does not resolve just yields an empty string
// instead of guessing an offset into this exact game build. IsFaulted alone
// only says a dispatch failed, not why; this is what would let a rate-limit
// or validation message distinguish itself from a bare transient failure.
std::string read_task_exception_message(void *task) {
    if (!task || !object_get_class || !class_get_method_from_name) return {};
    void *task_class = object_get_class(task);
    void *get_exception_method = task_class ? class_get_method_from_name(task_class, "get_Exception", 0) : nullptr;
    auto get_exception = get_exception_method
            ? reinterpret_cast<ObjectGetter>(*reinterpret_cast<void **>(get_exception_method)) : nullptr;
    void *exception = get_exception ? get_exception(task, nullptr) : nullptr;
    if (!exception) return {};
    void *exception_class = object_get_class(exception);
    const char *exception_class_name = exception_class && class_get_name ? class_get_name(exception_class) : nullptr;
    void *message_method = exception_class ? class_get_method_from_name(exception_class, "get_Message", 0) : nullptr;
    void *message_impl = message_method && object_get_virtual_method
            ? object_get_virtual_method(exception, message_method) : message_method;
    auto get_message = message_impl
            ? reinterpret_cast<ObjectGetter>(*reinterpret_cast<void **>(message_impl)) : nullptr;
    void *message = get_message ? get_message(exception, nullptr) : nullptr;
    std::string text = read_string(message);
    if (exception_class_name && !text.empty()) return std::string(exception_class_name) + ": " + text;
    if (exception_class_name) return exception_class_name;
    return text;
}

void append_rpc_fault_diagnostics(const std::string &task_id, const char *kind, bool faulted,
                                  int64_t elapsed_ms, int64_t ms_since_previous_attempt,
                                  int consecutive_faults, const std::string &exception_text) {
    FILE *file = std::fopen(dispatch_rpc_fault_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%s\t%d\t%" PRId64 "\t%" PRId64 "\t%d\t%s\n",
                 now_ms(), task_id.c_str(), kind ? kind : "", faulted ? 1 : 0, elapsed_ms,
                 ms_since_previous_attempt, consecutive_faults, exception_text.c_str());
    std::fclose(file);
    chmod(dispatch_rpc_fault_path, 0644);
}

void append_dispatch_gate_block(const std::string &task_id, const char *kind, bool lock_held,
                                bool requested, bool has_data, bool has_picked, int picked_count,
                                double distance, double allowed_distance) {
    FILE *file = std::fopen(dispatch_gate_block_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%s\t%d\t%d\t%d\t%d\t%d\t%.2f\t%.1f\n",
                 now_ms(), task_id.c_str(), kind ? kind : "", lock_held ? 1 : 0, requested ? 1 : 0,
                 has_data ? 1 : 0, has_picked ? 1 : 0, picked_count, distance, allowed_distance);
    std::fclose(file);
    chmod(dispatch_gate_block_path, 0644);
}

void append_dispatch_tick_trace(const std::string &batch_target, bool target_seen_raw,
                                int raw_count, int filtered_count) {
    FILE *file = std::fopen(dispatch_tick_trace_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%d\t%d\t%d\n",
                 now_ms(), batch_target.c_str(), target_seen_raw ? 1 : 0, raw_count, filtered_count);
    std::fclose(file);
    chmod(dispatch_tick_trace_path, 0644);
}

std::string read_mode() {
    FILE *file = std::fopen(mode_path, "r");
    if (!file) return "diag";
    char value[32]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    std::string result(value);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) {
        result.pop_back();
    }
    return result;
}

bool network_available() {
    ifaddrs *interfaces{};
    if (getifaddrs(&interfaces) != 0) return false;
    bool available = false;
    for (ifaddrs *item = interfaces; item; item = item->ifa_next) {
        if (!item->ifa_addr || item->ifa_addr->sa_family != AF_INET) continue;
        const unsigned flags = item->ifa_flags;
        if ((flags & IFF_UP) && !(flags & IFF_LOOPBACK)) {
            available = true;
            break;
        }
    }
    freeifaddrs(interfaces);
    return available;
}

bool is_planting() {
    if (!planting_controller) return false;
    void *sender = *reinterpret_cast<void **>(static_cast<uint8_t *>(planting_controller) + kPlantingSenderOffset);
    return sender && *reinterpret_cast<bool *>(static_cast<uint8_t *>(sender) + kChangeSenderItemOffset);
}

void write_planting_control_status(const std::string &mode, bool planting,
                                   const char *action, const std::string &petal_id = {}) {
    FILE *file = std::fopen(planting_control_status_path, "w");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%d\t%d\t%s\t%s\n", now_ms(), mode.c_str(),
                 planting ? 1 : 0, planting_control_owned ? 1 : 0,
                 action, petal_id.empty() ? "-" : petal_id.c_str());
    std::fclose(file);
    chmod(planting_control_status_path, 0644);
}

void maybe_dismiss_planting_result_dialog() {
    if (!planting_result_auto_close_pending || !planting_result_dialog || !close_planting_result_dialog) return;
    const long long now = now_ms();
    // Only a dialog created after this module-owned stop request is eligible.
    // This prevents a later controller action from closing a player's older,
    // manually-created result dialog.
    if (planting_result_dialog_seen_ms < planting_result_stop_requested_ms || now < planting_result_close_after_ms) return;
    void *task = close_planting_result_dialog(planting_result_dialog, nullptr);
    if (task && gchandle_new) gchandle_new(task, false);
    LOGI("[PLANTING-CONTROL] auto-dismissed owned result dialog task=%p", task);
    planting_result_auto_close_pending = false;
}

// "on" and "off" are explicit commands from Control Center.  Anything
// else, including a missing file, is passive observation. This protects a
// manual planting session from being stopped merely because the controller
// has not been installed/configured yet.
std::string read_planting_control_mode() {
    FILE *file = std::fopen(planting_control_mode_path, "r");
    if (!file) return "observe";
    char value[16]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    std::string result(value);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) result.pop_back();
    if (result == "on") return "on";
    return result == "off" ? "off" : "observe";
}

void maybe_manage_planting() {
    maybe_dismiss_planting_result_dialog();
    const std::string mode = read_planting_control_mode();
    const bool planting = is_planting();
    if (mode != planting_control_last_mode) {
        planting_control_last_mode = mode;
        planting_control_start_attempted = false;
        planting_control_stop_attempted = false;
    }
    if (!planting_controller || !get_planting_flower_petal_id ||
        !start_planting_with_confirmation || !stop_planting_with_confirmation) {
        planting_control_last_action = "controller-unavailable";
        write_planting_control_status(mode, planting, planting_control_last_action.c_str());
        return;
    }
    if (mode == "on") {
        if (planting) {
            // Already active can be either our own session or a player-owned
            // one. In the latter case leave ownership false so a later off
            // never shuts down the player's session.
            planting_control_last_action = planting_control_owned ? "active-owned" : "active-external";
            write_planting_control_status(mode, true, planting_control_last_action.c_str());
            return;
        }
        if (!planting_control_start_attempted) {
            void *petal = get_planting_flower_petal_id(planting_controller, nullptr);
            const std::string petal_id = utf8_string(petal);
            if (!petal || petal_id.empty()) {
                planting_control_start_attempted = true;
                planting_control_last_action = "no-selected-petal";
                write_planting_control_status(mode, false, planting_control_last_action.c_str());
                return;
            }
            // false deliberately suppresses the UI confirmation dialog; the
            // caller already made the explicit control-file decision. The
            // game controller still validates petals, inventory and location.
            void *task = start_planting_with_confirmation(planting_controller, petal, false, nullptr);
            if (task && gchandle_new) {
                planting_control_pending_task = task;
                planting_control_pending_task_handle = gchandle_new(task, false);
            }
            planting_control_start_attempted = true;
            planting_control_owned = true;
            planting_control_last_action = task ? "start-requested" : "start-no-task";
            LOGI("[PLANTING-CONTROL] start requested petal=%s task=%p", petal_id.c_str(), task);
            write_planting_control_status(mode, false, planting_control_last_action.c_str(), petal_id);
            return;
        }
        write_planting_control_status(mode, false, planting_control_last_action.c_str());
        return;
    }
    if (mode == "off" && planting_control_owned && planting && !planting_control_stop_attempted) {
        void *task = stop_planting_with_confirmation(planting_controller, false, nullptr);
        if (task && gchandle_new) {
            planting_control_pending_task = task;
            planting_control_pending_task_handle = gchandle_new(task, false);
        }
        planting_control_stop_attempted = true;
        // StopPlantingWithConfirmationAsync ends with the game's own result
        // dialog. Defer closing until its real Start() hook captures the new
        // instance, then call that dialog's own close handler rather than a
        // screen coordinate or Accessibility gesture.
        planting_result_stop_requested_ms = now_ms();
        planting_result_close_after_ms = planting_result_stop_requested_ms + 1000;
        planting_result_auto_close_pending = true;
        planting_control_last_action = task ? "stop-requested" : "stop-no-task";
        LOGI("[PLANTING-CONTROL] stop requested task=%p", task);
        write_planting_control_status(mode, true, planting_control_last_action.c_str());
        return;
    }
    if (mode == "off" && planting_control_owned && !planting) {
        planting_control_owned = false;
        planting_control_last_action = "stopped";
    } else if (mode == "off" && !planting_control_owned) {
        planting_control_last_action = planting ? "manual-session-preserved" : "off";
    } else if (mode == "observe") {
        planting_control_last_action = planting ? "observed-active" : "observe";
    }
    write_planting_control_status(mode, planting, planting_control_last_action.c_str());
}

bool current_location(double &latitude, double &longitude) {
    void *candidates[3]{location_controller, nullptr, nullptr};
    if (interaction_settings) {
        candidates[1] = *reinterpret_cast<void **>(static_cast<uint8_t *>(interaction_settings) + kLocationControllerOffset);
    }
    if (planting_controller) {
        candidates[2] = *reinterpret_cast<void **>(static_cast<uint8_t *>(planting_controller) + kPlantingLocationControllerOffset);
    }
    for (void *controller : candidates) {
        if (!controller) continue;
        auto *raw = static_cast<uint8_t *>(controller) + kRawLocationOffset;
        const double candidate_latitude = *reinterpret_cast<double *>(raw);
        const double candidate_longitude = *reinterpret_cast<double *>(raw + sizeof(double));
        if (std::isfinite(candidate_latitude) && std::isfinite(candidate_longitude) &&
            std::abs(candidate_latitude) > 0.0001 && std::abs(candidate_longitude) > 0.0001 &&
            std::abs(candidate_latitude) <= 90.0 && std::abs(candidate_longitude) <= 180.0) {
            latitude = candidate_latitude;
            longitude = candidate_longitude;
            return true;
        }
    }
    // The Magisk service rewrites this file every two seconds; it is used only
    // when the version-specific in-memory layout is unknown.  Refuse a stale
    // one: on 2026-08-29 a CRLF service.sh never started its loop, so this file
    // sat unchanged for over an hour and pinned the reported position ~90 m
    // from the player.  Distance gates then passed or failed on a location that
    // had not been true for hours, which is exactly the kind of silent wrong
    // answer this module must not give.  Without a fresh fix, fail closed and
    // let the caller treat the distance as unknown.
    struct stat gps_stat {};
    if (::stat(system_gps_path, &gps_stat) != 0) return false;
    const long long age_seconds =
            static_cast<long long>(time(nullptr)) - static_cast<long long>(gps_stat.st_mtime);
    if (age_seconds < 0 || age_seconds > kSystemGpsMaxAgeSeconds) {
        if (age_seconds != last_stale_gps_age_logged) {
            last_stale_gps_age_logged = age_seconds;
            LOGW("[GPS] ignoring stale %s (%lld s old); is service.sh running?",
                 system_gps_path, age_seconds);
        }
        return false;
    }
    FILE *fallback = std::fopen(system_gps_path, "r");
    if (!fallback) return false;
    const int read = std::fscanf(fallback, "%lf\t%lf", &latitude, &longitude);
    std::fclose(fallback);
    return read == 2 && std::isfinite(latitude) && std::isfinite(longitude) &&
           std::abs(latitude) > 0.0001 && std::abs(longitude) > 0.0001 &&
           std::abs(latitude) <= 90.0 && std::abs(longitude) <= 180.0;
}

void load_test_target() {
    if (target_loaded) return;
    target_loaded = true;
    FILE *file = std::fopen(target_path, "r");
    if (!file) return;
    char id[520]{};
    double latitude{}, longitude{};
    if (std::fscanf(file, "%519s\t%lf\t%lf", id, &latitude, &longitude) == 3) {
        FlowerRecord &record = flowers[id];
        record.latitude = latitude;
        record.longitude = longitude;
        record.state = 3;
        record.wilting_ms = 0;
        record.received = false;
        LOGI("[NECTAR-DIAG] fallback target loaded id=%s lat=%.7f lng=%.7f", id, latitude, longitude);
    }
    std::fclose(file);
}

double distance_metres(double lat1, double lng1, double lat2, double lng2) {
    constexpr double kEarthRadius = 6371000.0;
    constexpr double kRadians = 3.14159265358979323846 / 180.0;
    const double p1 = lat1 * kRadians;
    const double p2 = lat2 * kRadians;
    const double dp = (lat2 - lat1) * kRadians;
    const double dl = (lng2 - lng1) * kRadians;
    const double a = std::sin(dp / 2) * std::sin(dp / 2) +
                     std::cos(p1) * std::cos(p2) * std::sin(dl / 2) * std::sin(dl / 2);
    return kEarthRadius * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

bool resolve_request_class() {
    if (request_class) return true;
    if (!game_domain || !domain_get_assemblies || !assembly_get_image || !class_from_name) return false;
    size_t count{};
    const void **assemblies = domain_get_assemblies(game_domain, &count);
    for (size_t index = 0; assemblies && index < count; ++index) {
        void *image = assembly_get_image(assemblies[index]);
        void *klass = class_from_name(image, "Ichigo.Proto", "ClaimPoiFlowerVisitRewardRequestProto");
        if (klass) {
            request_class = klass;
            LOGI("[NECTAR] request class resolved from assembly index=%zu", index);
            return true;
        }
    }
    LOGE("[NECTAR] request class was not found");
    return false;
}

const char *failed_reason_name(int reason) {
    switch (reason) {
        case 0: return "NONE";
        case 1: return "INVENTORY_FULL";
        case 2: return "ALREADY_REWARDED";
        case 3: return "NOT_BLOOMING";
        default: return "UNKNOWN";
    }
}

void append_claim_log(const char *result, int failed_reason, int reward_case, int amount,
                      int honey_type, int flower_kind) {
    FILE *file = std::fopen(claim_log_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%s\t%.7f\t%.7f\t%.1f\t%d\t%s\t%d\t%d\t%d\t%d\n",
                 now_ms(), result, pending_id.c_str(), pending_gps_lat, pending_gps_lng,
                 pending_distance, failed_reason, failed_reason_name(failed_reason), reward_case,
                 amount, honey_type, flower_kind);
    std::fclose(file);
    chmod(claim_log_path, 0644);
}

void clear_pending_task() {
    // Keep completed Task handles rooted until the game process exits. Releasing a
    // freshly completed async Task here races an IL2CPP continuation in v150 and
    // can crash the Unity main thread. The number of claim Tasks per session is
    // small, so retaining them is the safer trade-off.
    pending_task_handle = 0;
    pending_task = nullptr;
    pending_id.clear();
}

void poll_pending_task() {
    if (!pending_task || !task_is_completed || !task_is_completed(pending_task, nullptr)) return;
    const bool faulted = task_is_faulted && task_is_faulted(pending_task, nullptr);
    void *response = *reinterpret_cast<void **>(static_cast<uint8_t *>(pending_task) + 0x50);
    if (faulted || !response) {
        const char *result = faulted ? "RPC_FAULTED" : "NO_RESPONSE";
        last_result = result;
        append_claim_log(result, -1, 0, 0, 0, 0);
        LOGE("[NECTAR] claim completed id=%s result=%s", pending_id.c_str(), result);
        clear_pending_task();
        return;
    }

    auto *response_bytes = static_cast<uint8_t *>(response);
    const int failed_reason = *reinterpret_cast<int *>(response_bytes + 0x28);
    void *rewards = *reinterpret_cast<void **>(response_bytes + 0x20);
    int reward_count{};
    void *array{};
    if (rewards) {
        auto *repeated_bytes = static_cast<uint8_t *>(rewards);
        array = *reinterpret_cast<void **>(repeated_bytes + 0x10);
        reward_count = *reinterpret_cast<int *>(repeated_bytes + 0x18);
        if (reward_count < 0 || reward_count > 32) reward_count = 0;
    }
    const char *result = failed_reason == 0 ? "SUCCESS" : "REJECTED";
    last_result = std::string(result) + ":" + failed_reason_name(failed_reason);
    if (!array || reward_count == 0) {
        append_claim_log(result, failed_reason, 0, 0, 0, 0);
    } else {
        auto *array_bytes = static_cast<uint8_t *>(array);
        for (int index = 0; index < reward_count; ++index) {
            void *reward = *reinterpret_cast<void **>(array_bytes + 0x20 + index * sizeof(void *));
            if (!reward) continue;
            auto *reward_bytes = static_cast<uint8_t *>(reward);
            const int amount = *reinterpret_cast<int *>(reward_bytes + 0x18);
            void *reward_type = *reinterpret_cast<void **>(reward_bytes + 0x28);
            const int reward_case = *reinterpret_cast<int *>(reward_bytes + 0x30);
            int honey_type{}, flower_kind{};
            if (reward_case == 3 && reward_type) {
                auto *honey = static_cast<uint8_t *>(reward_type);
                honey_type = *reinterpret_cast<int *>(honey + 0x18);
                flower_kind = *reinterpret_cast<int *>(honey + 0x1C);
            }
            append_claim_log(result, failed_reason, reward_case, amount, honey_type, flower_kind);
            LOGI("[NECTAR] reward id=%s case=%d amount=%d honey=%d flower=%d",
                 pending_id.c_str(), reward_case, amount, honey_type, flower_kind);
        }
    }
    LOGI("[NECTAR] claim completed id=%s result=%s failedReason=%d rewards=%d",
         pending_id.c_str(), result, failed_reason, reward_count);
    clear_pending_task();
}

// StartExpeditionAsync() returns a plain Task with no response body, so the
// only thing worth reading is IsCompleted/IsFaulted -- the same read-only
// pattern poll_pending_task() already uses safely.  On 2026-08-29 a fruit
// dispatched from 0.2 m produced a "start requested" log line and never
// confirmed; a retry from the same spot minutes later confirmed in 15 s.
// Nothing distinguished the two in the log.  This makes that distinction
// visible: whether the RPC itself ever completed, and whether it faulted.
void poll_pending_expedition_task() {
    if (!pending_expedition_task) return;
    if (!task_is_completed || !task_is_completed(pending_expedition_task, nullptr)) {
        // A Task that never completes would otherwise watch forever.
        if (now_ms() - pending_expedition_task_started_ms > 30000) {
            LOGW("[DISPATCH-RPC] task=%s not completed after 30s; abandoning watch",
                 pending_expedition_task_id.c_str());
            pending_expedition_task = nullptr;
        }
        return;
    }
    const bool faulted = task_is_faulted && task_is_faulted(pending_expedition_task, nullptr);
    const int64_t elapsed_ms = now_ms() - pending_expedition_task_started_ms;
    // Best-effort only: read_task_exception_message() resolves everything
    // dynamically by name and returns empty on any unresolved step, so a
    // build where this does not work costs nothing beyond an empty column.
    const std::string exception_text = faulted ? read_task_exception_message(pending_expedition_task) : std::string();
    consecutive_expedition_rpc_faults = faulted ? consecutive_expedition_rpc_faults + 1 : 0;
    LOGI("[DISPATCH-RPC] task=%s completed faulted=%d elapsedMs=%" PRId64 " streak=%d exception=%s",
         pending_expedition_task_id.c_str(), faulted ? 1 : 0, elapsed_ms, consecutive_expedition_rpc_faults,
         exception_text.empty() ? "-" : exception_text.c_str());
    append_dispatch_history(faulted ? "start-rpc-faulted" : "start-rpc-completed",
                            pending_expedition_task_kind.c_str(), pending_expedition_task_id, elapsed_ms, 0);
    append_rpc_fault_diagnostics(pending_expedition_task_id, pending_expedition_task_kind.c_str(), faulted,
                                 elapsed_ms, pending_expedition_ms_since_previous_attempt,
                                 consecutive_expedition_rpc_faults, exception_text);
    // A faulted RPC never actually started the expedition, so the task
    // stays in the unfinished list forever -- neither of the two normal
    // release paths in write_dispatch_candidates (finish_ms becoming
    // non-zero, or the task disappearing from the projection) can ever
    // fire for it. Without this, armed mode (which has no equivalent of
    // batch mode's Java-side releaseNativeConfirmationLock()) gets stuck
    // reporting "no-dispatch" forever after a single faulted attempt,
    // until dispatch mode is toggled off and back on by hand. Release the
    // lock here instead, at the point the fault is actually detected, so
    // the very next tick can retry.
    if (faulted && dispatch_confirmation_pending_id == pending_expedition_task_id) {
        dispatch_confirmation_pending_id.clear();
        dispatch_confirmation_started_observed_ms = 0;
    }
    // Not releasing the GC handle mirrors clear_pending_task(): freeing a
    // just-completed Task raced an IL2CPP continuation on v150 and crashed
    // the main thread, and expeditions per session are few enough that
    // retaining the handle is the cheaper trade-off.
    pending_expedition_task = nullptr;
}

// Armed mode can have several StartExpeditionAsync calls in flight at once.
// Poll each independently; a completed successful RPC remains in the map
// until the inventory projection confirms that task, while a fault releases
// only its own id for a later retry.  Batch continues to use the single-task
// watcher above so its GPS controller cannot advance early.
void poll_armed_expedition_tasks() {
    for (auto it = armed_dispatches.begin(); it != armed_dispatches.end();) {
        ArmedDispatchInFlight &pending = it->second;
        if (!pending.task || pending.rpc_result_recorded) {
            ++it;
            continue;
        }
        if (!task_is_completed || !task_is_completed(pending.task, nullptr)) {
            if (now_ms() - pending.started_ms > 30000) {
                LOGW("[DISPATCH-RPC] armed task=%s not completed after 30s; retaining inventory watch",
                     it->first.c_str());
                // Do not re-start a request whose managed Task may still run.
                // Inventory confirmation remains the only release path.
                pending.task = nullptr;
            }
            ++it;
            continue;
        }
        const bool faulted = task_is_faulted && task_is_faulted(pending.task, nullptr);
        const int64_t elapsed_ms = now_ms() - pending.started_ms;
        const std::string exception_text = faulted ? read_task_exception_message(pending.task) : std::string();
        consecutive_expedition_rpc_faults = faulted ? consecutive_expedition_rpc_faults + 1 : 0;
        LOGI("[DISPATCH-RPC] armed task=%s completed faulted=%d elapsedMs=%" PRId64 " streak=%d exception=%s",
             it->first.c_str(), faulted ? 1 : 0, elapsed_ms, consecutive_expedition_rpc_faults,
             exception_text.empty() ? "-" : exception_text.c_str());
        append_dispatch_history(faulted ? "start-rpc-faulted" : "start-rpc-completed",
                                pending.kind.c_str(), it->first, elapsed_ms, 0);
        append_rpc_fault_diagnostics(it->first, pending.kind.c_str(), faulted,
                                     elapsed_ms, pending.ms_since_previous_attempt,
                                     consecutive_expedition_rpc_faults, exception_text);
        if (faulted) {
            // A faulted request leaves no expedition in the inventory, so its
            // own entry must be released. Other armed starts stay protected.
            it = armed_dispatches.erase(it);
        } else {
            pending.rpc_result_recorded = true;
            ++it;
        }
    }
}

void write_status(const std::string &mode, bool online, bool planting) {
    double latitude{}, longitude{};
    const bool has_location = current_location(latitude, longitude);
    FILE *file = std::fopen(status_path, "w");
    if (!file) {
        LOGE("[NECTAR] unable to write status path=%s errno=%d", status_path, errno);
        return;
    }
    std::fprintf(file, "%lld\t%s\t%d\t%d\t%d\t%.7f\t%.7f\t%zu\t%d\t%s\n",
                 now_ms(), mode.c_str(), planting ? 1 : 0, online ? 1 : 0,
                 has_location ? 1 : 0, latitude, longitude, flowers.size(),
                 rpc_manager ? 1 : 0, last_result.c_str());
    std::fclose(file);
    chmod(status_path, 0644);
}

void maybe_claim() {
    const long long current = now_ms();
    if (current - last_tick_ms < 1000) return;
    last_tick_ms = current;
    const std::string mode = read_mode();
    poll_pending_task();
    poll_pending_expedition_task();
    poll_armed_expedition_tasks();
    const bool online = network_available();
    const bool planting = is_planting();
    write_status(mode, online, planting);
    maybe_manage_planting();
    maybe_return_tasks();
    if (mode != "test_once" && mode != "auto") return;
    if (mode == "test_once" && test_once_sent) return;
    if (pending_task) return;
    load_test_target();
    // Claiming a visible big flower only needs a live game RPC session and a
    // location.  Flower planting is unrelated and must not gate collection.
    if (!rpc_manager || !online) {
        if (current - last_status_ms >= 5000) {
            last_status_ms = current;
            LOGI("[NECTAR-DIAG] waiting rpc=%d online=%d planting=%d flowers=%zu mode=%s",
                 rpc_manager ? 1 : 0, online ? 1 : 0, planting ? 1 : 0, flowers.size(), mode.c_str());
        }
        return;
    }
    if (current - last_claim_ms < 3000) return;

    double player_lat{}, player_lng{};
    if (!current_location(player_lat, player_lng)) {
        if (current - last_status_ms >= 5000) {
            last_status_ms = current;
            LOGI("[NECTAR-DIAG] waiting for LocationController flowers=%zu", flowers.size());
        }
        return;
    }
    for (auto &entry : flowers) {
        FlowerRecord &flower = entry.second;
        if (flower.attempted || flower.received || flower.state != 3) continue;
        if (flower.wilting_ms > 0 && flower.wilting_ms <= current) continue;
        const double distance = distance_metres(player_lat, player_lng, flower.latitude, flower.longitude);
        // GPS updates and map-object positions can differ by a few metres.
        // Permit a bounded tolerance above the nominal 100 m collection radius.
        if (distance > 120.0) continue;
        if (!resolve_request_class() || !object_new || !string_new || !request_constructor ||
            !request_set_map_object_id || !request_set_include_failure || !send_claim) return;
        void *request = object_new(request_class);
        if (!request) return;
        request_constructor(request, nullptr);
        void *managed_id = string_new(entry.first.c_str());
        request_set_map_object_id(request, managed_id, nullptr);
        request_set_include_failure(request, true, nullptr);
        void *task = send_claim(rpc_manager, request, nullptr, 2, nullptr);
        flower.attempted = true;
        last_claim_ms = current;
        if (mode == "test_once") test_once_sent = true;
        if (task && gchandle_new) {
            pending_task = task;
            pending_task_handle = gchandle_new(task, false);
            pending_id = entry.first;
            pending_gps_lat = player_lat;
            pending_gps_lng = player_lng;
            pending_distance = distance;
            last_result = "PENDING";
        }
        LOGI("[NECTAR] claim sent id=%s distance=%.1fm gps=%.7f,%.7f task=%p mode=%s",
             entry.first.c_str(), distance, player_lat, player_lng, task, mode.c_str());
        return;
    }
    if (current - last_status_ms >= 5000) {
        last_status_ms = current;
        LOGI("[NECTAR-DIAG] no eligible flower gps=%.7f,%.7f flowers=%zu", player_lat,
             player_lng, flowers.size());
    }
}

void log_flower(void *map_object) {
    if (!map_object || !object_get_class || !class_get_name) return;
    void *klass = object_get_class(map_object);
    const char *class_name = klass ? class_get_name(klass) : nullptr;
    if (!class_name || std::strcmp(class_name, "MapPoiFlower") != 0) return;
    interaction_settings = *reinterpret_cast<void **>(static_cast<uint8_t *>(map_object) + kInteractionSettingsOffset);

    auto *bytes = static_cast<uint8_t *>(map_object);
    void *map_proto = *reinterpret_cast<void **>(bytes + kInitialMapObjectProtoOffset);
    if (!map_proto) return;
    auto *proto_bytes = static_cast<uint8_t *>(map_proto);
    const std::string id = read_string(*reinterpret_cast<void **>(proto_bytes + kMapObjectIdOffset));
    void *point = *reinterpret_cast<void **>(proto_bytes + kMapObjectPointOffset);
    void *flower = *reinterpret_cast<void **>(proto_bytes + kMapObjectPayloadOffset);
    if (id.empty() || !point || !flower) return;

    auto *point_bytes = static_cast<uint8_t *>(point);
    auto *flower_bytes = static_cast<uint8_t *>(flower);
    const double latitude = *reinterpret_cast<double *>(point_bytes + kPointLatitudeOffset);
    const double longitude = *reinterpret_cast<double *>(point_bytes + kPointLongitudeOffset);
    const int state = *reinterpret_cast<int *>(flower_bytes + kFlowerStateOffset);
    const long long remaining = *reinterpret_cast<long long *>(flower_bytes + kFlowerRemainingOffset);
    const long long wilting_ms = *reinterpret_cast<long long *>(flower_bytes + kFlowerWiltingTimeOffset);
    const long long bloomed_ms = *reinterpret_cast<long long *>(flower_bytes + kFlowerBloomedTimeOffset);
    const bool received = *reinterpret_cast<bool *>(flower_bytes + kFlowerRewardReceivedOffset);
    FlowerRecord &record = flowers[id];
    record.latitude = latitude;
    record.longitude = longitude;
    record.state = state;
    record.wilting_ms = wilting_ms;
    record.received = received;

    char state_key[160];
    std::snprintf(state_key, sizeof(state_key), "%d:%lld:%lld:%lld:%d", state, remaining,
                  bloomed_ms, wilting_ms, received ? 1 : 0);
    auto previous = last_flower_state.find(id);
    if (previous != last_flower_state.end() && previous->second == state_key) return;
    last_flower_state[id] = state_key;
    LOGI("[NECTAR-DIAG] id=%s lat=%.7f lng=%.7f state=%d remaining=%lld bloomed=%lld wilting=%lld received=%d",
         id.c_str(), latitude, longitude, state, remaining, bloomed_ms, wilting_ms, received ? 1 : 0);
    FILE *file = std::fopen(flower_log_path, "a");
    if (file) {
        std::fprintf(file, "%lld\t%s\t%.7f\t%.7f\t%d\t%lld\t%lld\t%lld\t%d\n",
                     now_ms(), id.c_str(), latitude, longitude, state, remaining, bloomed_ms,
                     wilting_ms, received ? 1 : 0);
        std::fclose(file);
    }
}

void *hooked_get_flower_proto(void *flower, void *method_info) {
    void *result = original_get_flower_proto ? original_get_flower_proto(flower, method_info) : nullptr;
    log_flower(flower);
    return result;
}
void hooked_construct_proto_object(void *self, void *proto, void *method_info) {
    if (original_construct_proto_object) original_construct_proto_object(self, proto, method_info);
    log_flower(self);
}
void hooked_update_proto_object(void *self, void *proto, void *method_info) {
    if (original_update_proto_object) original_update_proto_object(self, proto, method_info);
    log_flower(self);
}
void hooked_flower_model_updated(void *self, void *method_info) {
    if (original_flower_model_updated) original_flower_model_updated(self, method_info);
    log_flower(self);
    // v152 registers live flowers without consistently calling the manager's
    // Update method.  Run the same mode-gated heartbeat here so diagnostic
    // status and opt-in automation both see those observations.
    maybe_claim();
    maybe_return_tasks();
}
void hooked_rpc_manager_constructor(void *self, void *method_info) {
    if (original_rpc_manager_constructor) original_rpc_manager_constructor(self, method_info);
    rpc_manager = self;
    LOGI("[NECTAR-DIAG] RpcManager constructed this=%p", self);
}
void hooked_rpc_manager_logged_in(void *self, void *server_url, void *player_id,
                                  void *background_token, void *method_info) {
    rpc_manager = self;
    LOGI("[NECTAR-DIAG] RpcManager logged in this=%p", self);
    if (original_rpc_manager_logged_in) original_rpc_manager_logged_in(self, server_url, player_id, background_token, method_info);
    initialize_runtime_metadata();
}
void hooked_register_map_object(void *self, void *map_object, int tag, void *method_info) {
    if (original_register_map_object) original_register_map_object(self, map_object, tag, method_info);
    // Map objects are registered only after the live game world is active.
    // This is a reliable safe point when the earlier login callback occurred
    // before our direct hooks had been installed.
    initialize_runtime_metadata();
    log_flower(map_object);
    maybe_claim();
    maybe_return_tasks();
}

std::string read_return_mode() {
    FILE *file = std::fopen(return_mode_path, "r");
    if (!file) return "dry-run";
    char value[32]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    std::string result(value);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) result.pop_back();
    return result.empty() ? "dry-run" : result;
}

// Dispatch remains fail-closed until the control centre explicitly writes a
// recognised mode. "armed" is the original nearby one-shot mode; "batch"
// additionally requires an exact task id plus a fresh arrival proof.
std::string read_dispatch_mode() {
    FILE *file = std::fopen(dispatch_mode_path, "r");
    if (!file) return "off";
    char value[16]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    if (std::strncmp(value, "armed", 5) == 0) return "armed";
    return std::strncmp(value, "batch", 5) == 0 ? "batch" : "off";
}

// The filter is fail-open to preserve every existing armed workflow: only an
// exact recognised value narrows candidates. Batch mode never consults it.
std::string read_dispatch_kind_filter() {
    FILE *file = std::fopen(dispatch_kinds_path, "r");
    if (!file) return "all";
    char value[16]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    std::string result(value);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) result.pop_back();
    return (result == "seed" || result == "fruit" || result == "gift" || result == "farm") ? result : "all";
}

std::string read_dispatch_target() {
    FILE *file = std::fopen(dispatch_target_path, "r");
    if (!file) return {};
    char value[256]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    std::string result(value);
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' || result.back() == ' ')) result.pop_back();
    return result;
}

bool dispatch_target_is_ready(const std::string &target_id, long long observed_ms) {
    FILE *file = std::fopen(dispatch_ready_path, "r");
    if (!file) return false;
    char id[256]{};
    long long ready_ms{};
    const int fields = std::fscanf(file, "%255[^\t]\t%lld", id, &ready_ms);
    std::fclose(file);
    // A readiness proof is intentionally short-lived, so a stale file cannot
    // arm an expedition after the joystick was moved elsewhere.
    return fields == 2 && target_id == id && ready_ms > 0 &&
           observed_ms >= ready_ms && observed_ms - ready_ms <= 30000;
}

void append_dispatch_history(const char *event, const char *kind, const std::string &task_id,
                             int64_t duration_ms, int picked_count) {
    const long long cutoff = now_ms() - 24LL * 60 * 60 * 1000;
    const std::string temporary = std::string(dispatch_history_path) + ".tmp";
    std::ifstream source(dispatch_history_path);
    std::ofstream retained(temporary, std::ios::trunc);
    std::string line;
    while (source && std::getline(source, line)) {
        char *end{};
        const long long timestamp = std::strtoll(line.c_str(), &end, 10);
        if (end != line.c_str() && timestamp >= cutoff) retained << line << '\n';
    }
    source.close(); retained.close();
    if (std::rename(temporary.c_str(), dispatch_history_path) != 0) std::remove(temporary.c_str());
    FILE *file = std::fopen(dispatch_history_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%s\t%s\t%" PRId64 "\t%d\n", now_ms(), event, kind,
                 task_id.c_str(), duration_ms, picked_count);
    std::fclose(file);
    chmod(dispatch_history_path, 0644);
}

// The policy defaults to preserving postcards. Only the explicit "discard"
// value changes the boolean passed to the game's native completion API.
bool return_discard_postcard() {
    FILE *file = std::fopen(return_postcard_policy_path, "r");
    if (!file) return false;
    char value[32]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    return std::strncmp(value, "discard", 7) == 0;
}

int return_batch_limit() {
    FILE *file = std::fopen(return_batch_limit_path, "r");
    if (!file) return 5;
    char value[16]{};
    std::fgets(value, sizeof(value), file);
    std::fclose(file);
    const long parsed = std::strtol(value, nullptr, 10);
    return parsed >= 1 && parsed <= 5 ? static_cast<int>(parsed) : 5;
}

// One-shot native validation.  This is intentionally separate from dry-run:
// it can dispatch at most one server-backed completion in a process lifetime.
void maybe_dispatch_one_return_task() {
    if (read_return_mode() != "one" || !return_action_manager ||
        !return_inventory_manager || !original_get_pikmin_task_list || !original_complete_pikmin_task ||
        !get_inventory_item_id || !get_pikmin_task_proto || !get_task_finish_time_ms) return;
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) return;
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > kMaxPikminTaskListCount) return;
    if (return_one_waiting) {
        const bool pending_seen = task_list_contains_id(items, count, return_batch_pending_id);
        if (count < return_one_baseline_count || !pending_seen) {
            return_one_waiting = false;
            write_return_status("one-confirmed", count, 1, false, return_discard_postcard());
            return_batch_pending_id.clear();
            return_batch_pending_reward.clear();
        }
        return;
    }
    if (return_one_dispatched) return;
    const long long now = now_ms();
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!task) continue;
        void *proto = get_pikmin_task_proto(task, nullptr);
        const int64_t finish_ms = proto ? get_task_finish_time_ms(proto, nullptr) : 0;
        if (finish_ms <= 0 || finish_ms > now) continue;
        void *task_id = get_inventory_item_id(task, nullptr);
        if (!task_id) continue;
        return_one_dispatched = true;
        return_one_waiting = true;
        return_one_baseline_count = count;
        last_return_batch_mode = "one";
        return_batch_pending_id = utf8_string(task_id);
        return_batch_pending_reward = describe_return_reward(task);
        return_all_empty_reported = false;
        append_return_trace("native-dispatch-one", return_action_manager, task_id);
        const bool discard_postcard = return_discard_postcard();
        write_return_status("one-dispatched", count, 0, true, discard_postcard);
        void *async_task = original_complete_pikmin_task(return_action_manager, task_id, discard_postcard, nullptr);
        if (async_task && gchandle_new) gchandle_new(async_task, false);
        LOGI("[RETURN-DIAG] native one-shot dispatched task=%s async=%p",
             utf8_string(task_id).c_str(), async_task);
        return;
    }
}

void maybe_dispatch_return_batch() {
    const std::string mode = read_return_mode();
    const bool automatic_batch = mode == "batch" || mode == "all";
    if (!automatic_batch) {
        // Leaving batch mode explicitly re-arms the next batch, without
        // allowing a paused process to restart itself unexpectedly.
        if (last_return_batch_mode == "batch" || last_return_batch_mode == "all") {
            return_batch_waiting = false;
            return_batch_stopped = false;
            return_batch_baseline_count = 0;
            return_batch_completed = 0;
            return_batch_pending_id.clear();
            return_batch_pending_reward.clear();
        }
        last_return_batch_mode = mode;
        return;
    }
    if (last_return_batch_mode != mode) {
        return_batch_waiting = false;
        return_batch_stopped = false;
        return_batch_baseline_count = 0;
        return_batch_completed = 0;
        return_batch_pending_id.clear();
        return_batch_pending_reward.clear();
        last_return_batch_mode = mode;
    }
    if (return_batch_stopped || !return_action_manager ||
        !return_inventory_manager || !original_get_pikmin_task_list || !original_complete_pikmin_task ||
        !get_inventory_item_id || !get_pikmin_task_proto || !get_task_finish_time_ms) return;
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) return;
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > kMaxPikminTaskListCount) return;
    const long long now = now_ms();
    if (return_batch_waiting) {
        const bool pending_seen = task_list_contains_id(items, count, return_batch_pending_id);
        if (count < return_batch_baseline_count || !pending_seen) {
            return_batch_waiting = false;
            ++return_batch_completed;
            LOGI("[RETURN-DIAG] batch confirmed completed=%d", return_batch_completed);
            write_return_status("batch-confirmed", count, return_batch_completed, false,
                                return_discard_postcard());
            return_batch_pending_id.clear();
            return_batch_pending_reward.clear();
        } else if (now - return_batch_dispatched_ms > 60000) {
            return_batch_stopped = true;
            LOGE("[RETURN-DIAG] batch stopped: pending task still present after 60s id=%s count=%d baseline=%d",
                 return_batch_pending_id.c_str(), count, return_batch_baseline_count);
            write_return_status("batch-stopped-timeout", count, return_batch_completed, true,
                                return_discard_postcard());
        }
        return;
    }
    const int batch_limit = mode == "all" ? INT32_MAX : return_batch_limit();
    if (return_batch_completed >= batch_limit) {
        return_batch_stopped = true;
        LOGI("[RETURN-DIAG] batch paused after %d confirmed completions", batch_limit);
        write_return_status("batch-paused-limit", count, return_batch_completed, false,
                            return_discard_postcard());
        return;
    }
    for (int index = 0; index < count; ++index) {
        void *task = *reinterpret_cast<void **>(static_cast<uint8_t *>(items) + 0x20 + index * sizeof(void *));
        if (!task) continue;
        void *proto = get_pikmin_task_proto(task, nullptr);
        const int64_t finish_ms = proto ? get_task_finish_time_ms(proto, nullptr) : 0;
        if (finish_ms <= 0 || finish_ms > now) continue;
        void *task_id = get_inventory_item_id(task, nullptr);
        if (!task_id) continue;
        return_batch_baseline_count = count;
        return_batch_dispatched_ms = now;
        return_batch_waiting = true;
        return_batch_pending_id = utf8_string(task_id);
        return_batch_pending_reward = describe_return_reward(task);
        append_return_trace("native-dispatch-batch", return_action_manager, task_id);
        const bool discard_postcard = return_discard_postcard();
        write_return_status("batch-dispatched", count, return_batch_completed, true, discard_postcard);
        void *async_task = original_complete_pikmin_task(return_action_manager, task_id, discard_postcard, nullptr);
        if (async_task && gchandle_new) gchandle_new(async_task, false);
        LOGI("[RETURN-DIAG] batch dispatched task=%s async=%p", utf8_string(task_id).c_str(), async_task);
        return;
    }
    if (mode == "all" && !return_all_empty_reported) {
        return_all_empty_reported = true;
        LOGI("[RETURN-DIAG] all mode idle: no due tasks remain; it will re-arm when a task appears");
        write_return_status("all-complete-no-due", count, return_batch_completed, false,
                            return_discard_postcard());
    }
}
void maybe_return_tasks() {
    const long long current = now_ms();
    if (current - last_return_scheduler_ms < 1000) return;
    last_return_scheduler_ms = current;
    dry_run_return_tasks();
    maybe_dispatch_one_return_task();
    maybe_dispatch_return_batch();
}

void hooked_map_update(void *self, void *method_info) {
    if (original_map_update) original_map_update(self, method_info);
    maybe_claim();
    maybe_return_tasks();
}
void hooked_planting_result_dialog_start(void *self, void *method_info) {
    if (original_planting_result_dialog_start) original_planting_result_dialog_start(self, method_info);
    planting_result_dialog = self;
    planting_result_dialog_seen_ms = now_ms();
    LOGI("[PLANTING-CONTROL] result dialog observed self=%p", self);
}
void hooked_planting_init(void *self, void *method_info) {
    if (original_planting_init) original_planting_init(self, method_info);
    planting_controller = self;
    if (self && !planting_controller_metadata_logged && object_get_class) {
        planting_controller_metadata_logged = true;
        void *klass = object_get_class(self);
        log_class_methods("PlantingController", klass);
        void *petal_method = klass ? class_get_method_from_name(klass, "get_FlowerPetalId", 0) : nullptr;
        void *start_method = klass ? class_get_method_from_name(klass, "StartPlantingWithConfirmationAsync", 2) : nullptr;
        void *stop_method = klass ? class_get_method_from_name(klass, "StopPlantingWithConfirmationAsync", 1) : nullptr;
        get_planting_flower_petal_id = petal_method
                ? reinterpret_cast<GetManagedString>(*reinterpret_cast<void **>(petal_method)) : nullptr;
        start_planting_with_confirmation = start_method
                ? reinterpret_cast<StartPlantingWithConfirmation>(*reinterpret_cast<void **>(start_method)) : nullptr;
        stop_planting_with_confirmation = stop_method
                ? reinterpret_cast<StopPlantingWithConfirmation>(*reinterpret_cast<void **>(stop_method)) : nullptr;
        void *dialog_class = find_class("Niantic.Ichigo.Game.Flowers.PlantingResultDialog", "FlowerPlantingResultDialog");
        void *dialog_start_method = dialog_class ? class_get_method_from_name(dialog_class, "Start", 0) : nullptr;
        void *dialog_close_method = dialog_class ? class_get_method_from_name(dialog_class, "OnCloseButtonClicked", 0) : nullptr;
        close_planting_result_dialog = dialog_close_method
                ? reinterpret_cast<NoArgTask>(*reinterpret_cast<void **>(dialog_close_method)) : nullptr;
        if (dialog_start_method && !planting_result_hook_installed) {
            A64HookFunction(*reinterpret_cast<void **>(dialog_start_method),
                            reinterpret_cast<void *>(hooked_planting_result_dialog_start),
                            reinterpret_cast<void **>(&original_planting_result_dialog_start));
            planting_result_hook_installed = original_planting_result_dialog_start != nullptr;
        }
        LOGI("[PLANTING-CONTROL] methods petal=%p start=%p stop=%p",
             get_planting_flower_petal_id, start_planting_with_confirmation,
             stop_planting_with_confirmation);
    }
}
void hooked_planting_start(void *self, void *method_info) {
    if (original_planting_start) original_planting_start(self, method_info);
    planting_controller = self;
}
void hooked_planting_state_updated(void *self, void *method_info) {
    if (original_planting_state_updated) original_planting_state_updated(self, method_info);
    planting_controller = self;
    LOGI("[NECTAR-DIAG] planting=%d", is_planting() ? 1 : 0);
}
void hooked_location_controller_awake(void *self, void *method_info) {
    location_controller = self;
    if (original_location_controller_awake) original_location_controller_awake(self, method_info);
    LOGI("[NECTAR-DIAG] LocationController captured this=%p", self);
}

template <typename T>
void install_hook(uintptr_t base, uintptr_t rva, void *replacement, T &original) {
    A64HookFunction(reinterpret_cast<void *>(base + rva), replacement,
                    reinterpret_cast<void **>(&original));
}

void start(const char *game_data_dir) {
    void *handle{};
    for (int attempt = 0; attempt < 120 && !handle; ++attempt) {
        handle = xdl_open("libil2cpp.so", XDL_DEFAULT);
        if (!handle) sleep(1);
    }
    if (!handle) { LOGE("[NECTAR] libil2cpp.so was not loaded"); return; }

    domain_get = reinterpret_cast<DomainGet>(xdl_sym(handle, "il2cpp_domain_get", nullptr));
    domain_get_assemblies = reinterpret_cast<DomainGetAssemblies>(xdl_sym(handle, "il2cpp_domain_get_assemblies", nullptr));
    assembly_get_image = reinterpret_cast<AssemblyGetImage>(xdl_sym(handle, "il2cpp_assembly_get_image", nullptr));
    class_from_name = reinterpret_cast<ClassFromName>(xdl_sym(handle, "il2cpp_class_from_name", nullptr));
    class_get_method_from_name = reinterpret_cast<ClassGetMethodFromName>(
            xdl_sym(handle, "il2cpp_class_get_method_from_name", nullptr));
    class_get_methods = reinterpret_cast<ClassGetMethods>(xdl_sym(handle, "il2cpp_class_get_methods", nullptr));
    method_get_name = reinterpret_cast<MethodGetName>(xdl_sym(handle, "il2cpp_method_get_name", nullptr));
    method_get_param_count = reinterpret_cast<MethodGetParamCount>(xdl_sym(handle, "il2cpp_method_get_param_count", nullptr));
    object_new = reinterpret_cast<ObjectNew>(xdl_sym(handle, "il2cpp_object_new", nullptr));
    array_new = reinterpret_cast<ArrayNew>(xdl_sym(handle, "il2cpp_array_new", nullptr));
    string_new = reinterpret_cast<StringNew>(xdl_sym(handle, "il2cpp_string_new", nullptr));
    gchandle_new = reinterpret_cast<GcHandleNew>(xdl_sym(handle, "il2cpp_gchandle_new", nullptr));
    object_get_class = reinterpret_cast<ObjectGetClass>(xdl_sym(handle, "il2cpp_object_get_class", nullptr));
    object_get_virtual_method = reinterpret_cast<ObjectGetVirtualMethod>(xdl_sym(handle, "il2cpp_object_get_virtual_method", nullptr));
    class_get_name = reinterpret_cast<ClassGetName>(xdl_sym(handle, "il2cpp_class_get_name", nullptr));
    Dl_info info{};
    if (!domain_get || !object_get_class || !class_get_name || !dladdr(reinterpret_cast<void *>(domain_get), &info)) {
        LOGE("[NECTAR] unable to resolve IL2CPP runtime symbols"); return;
    }
    const auto base = reinterpret_cast<uintptr_t>(info.dli_fbase);
    std::snprintf(flower_log_path, sizeof(flower_log_path), "%s/files/nectar_flowers.tsv", game_data_dir);
    std::snprintf(mode_path, sizeof(mode_path), "%s/files/nectar_rpc_mode.txt", game_data_dir);
    std::snprintf(target_path, sizeof(target_path), "%s/files/nectar_rpc_target.tsv", game_data_dir);
    std::snprintf(status_path, sizeof(status_path), "%s/files/nectar_status.tsv", game_data_dir);
    std::snprintf(claim_log_path, sizeof(claim_log_path), "%s/files/nectar_claims.tsv", game_data_dir);
    std::snprintf(system_gps_path, sizeof(system_gps_path), "%s/files/nectar_system_gps.tsv", game_data_dir);
    std::snprintf(return_trace_path, sizeof(return_trace_path), "%s/files/return_rpc_trace.tsv", game_data_dir);
    std::snprintf(return_history_path, sizeof(return_history_path), "%s/files/return_reward_history.tsv", game_data_dir);
    // These paths are shared with the root-protected Control Center.  Keeping
    // the native reader and controller writer identical prevents a UI setting
    // from silently remaining in dry-run mode.
    std::snprintf(return_mode_path, sizeof(return_mode_path), "/data/local/tmp/pikmin-return-mode.txt");
    std::snprintf(return_postcard_policy_path, sizeof(return_postcard_policy_path), "/data/local/tmp/pikmin-return-postcard-policy.txt");
    std::snprintf(return_status_path, sizeof(return_status_path), "%s/files/return_rpc_status.tsv", game_data_dir);
    std::snprintf(return_batch_limit_path, sizeof(return_batch_limit_path), "/data/local/tmp/pikmin-return-batch-limit.txt");
    std::snprintf(compatibility_path, sizeof(compatibility_path), "%s/files/compatibility_status.tsv", game_data_dir);
    std::snprintf(dispatch_candidates_path, sizeof(dispatch_candidates_path), "%s/files/dispatch_candidates.tsv", game_data_dir);
    std::snprintf(dispatch_status_path, sizeof(dispatch_status_path), "%s/files/dispatch_probe_status.tsv", game_data_dir);
    std::snprintf(dispatch_mode_path, sizeof(dispatch_mode_path), "/data/local/tmp/pikmin-dispatch-mode.txt");
    std::snprintf(dispatch_kinds_path, sizeof(dispatch_kinds_path), "/data/local/tmp/pikmin-dispatch-kinds.txt");
    std::snprintf(dispatch_target_path, sizeof(dispatch_target_path), "/data/local/tmp/pikmin-dispatch-target.txt");
    std::snprintf(dispatch_ready_path, sizeof(dispatch_ready_path), "/data/local/tmp/pikmin-dispatch-ready.tsv");
    std::snprintf(dispatch_history_path, sizeof(dispatch_history_path), "%s/files/dispatch_history.tsv", game_data_dir);
    std::snprintf(dispatch_diagnostics_path, sizeof(dispatch_diagnostics_path), "%s/files/dispatch_diagnostics.tsv", game_data_dir);
    std::snprintf(dispatch_rpc_fault_path, sizeof(dispatch_rpc_fault_path), "%s/files/dispatch_rpc_faults.tsv", game_data_dir);
    std::snprintf(dispatch_gate_block_path, sizeof(dispatch_gate_block_path), "%s/files/dispatch_gate_blocks.tsv", game_data_dir);
    std::snprintf(dispatch_tick_trace_path, sizeof(dispatch_tick_trace_path), "%s/files/dispatch_tick_trace.tsv", game_data_dir);
    std::snprintf(planting_control_mode_path, sizeof(planting_control_mode_path), "/data/local/tmp/pikmin-planting-mode.txt");
    std::snprintf(planting_control_status_path, sizeof(planting_control_status_path), "%s/files/planting_control_status.tsv", game_data_dir);
    request_constructor = reinterpret_cast<RequestConstructor>(base + kRequestConstructorRva);
    request_set_map_object_id = reinterpret_cast<RequestSetString>(base + kRequestSetMapObjectIdRva);
    request_set_include_failure = reinterpret_cast<RequestSetBool>(base + kRequestSetIncludeFailureRva);
    send_claim = reinterpret_cast<SendClaim>(base + kSendClaimRva);
    struct stat il2cpp_stat{};
    if (!info.dli_fname || stat(info.dli_fname, &il2cpp_stat) != 0
            || il2cpp_stat.st_size != kExpectedIl2CppSize) {
        write_compatibility_status(false, info.dli_fname ? il2cpp_stat.st_size : 0);
        LOGE("[NECTAR] unsupported libil2cpp.so; hooks were not installed");
        return;
    }
    write_compatibility_status(true, il2cpp_stat.st_size);
    task_is_completed = reinterpret_cast<TaskBool>(base + kTaskIsCompletedRva);
    task_is_faulted = reinterpret_cast<TaskBool>(base + kTaskIsFaultedRva);

    install_hook(base, kRegisterMapObjectRva, reinterpret_cast<void *>(hooked_register_map_object), original_register_map_object);
    install_hook(base, kMapObjectManagerUpdateRva, reinterpret_cast<void *>(hooked_map_update), original_map_update);
    install_hook(base, kFlowerModelUpdatedRva, reinterpret_cast<void *>(hooked_flower_model_updated), original_flower_model_updated);
    install_hook(base, kRpcManagerConstructorRva, reinterpret_cast<void *>(hooked_rpc_manager_constructor), original_rpc_manager_constructor);
    install_hook(base, kRpcManagerLoggedInRva, reinterpret_cast<void *>(hooked_rpc_manager_logged_in), original_rpc_manager_logged_in);
    install_hook(base, kPlantingInitRva, reinterpret_cast<void *>(hooked_planting_init), original_planting_init);
    install_hook(base, kPlantingStartRva, reinterpret_cast<void *>(hooked_planting_start), original_planting_start);
    install_hook(base, kPlantingStateUpdatedRva, reinterpret_cast<void *>(hooked_planting_state_updated), original_planting_state_updated);
    install_hook(base, kLocationControllerAwakeRva, reinterpret_cast<void *>(hooked_location_controller_awake), original_location_controller_awake);
    LOGI("[NECTAR] v152 direct hooks installed base=%" PRIxPTR "; metadata waits for login", base);
}

}  // namespace

void hack_prepare(const char *game_data_dir, void *, size_t) {
    std::thread worker(start, game_data_dir);
    worker.detach();
}
