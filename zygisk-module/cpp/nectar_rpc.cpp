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
#include <ifaddrs.h>
#include <map>
#include <net/if.h>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <time.h>
#include <unistd.h>

namespace {

// Pikmin Bloom 151.0 / arm64-v8a. Regenerated from the installed APK's
// libil2cpp.so plus global-metadata.dat; nonmatching binaries fail closed.
constexpr off_t kExpectedIl2CppSize = 254191320;
constexpr uintptr_t kRegisterMapObjectRva = 0x59E4ADC;
constexpr uintptr_t kMapObjectManagerUpdateRva = 0x59E67D0;
constexpr uintptr_t kFlowerModelUpdatedRva = 0x59F79E4;
constexpr uintptr_t kRpcManagerConstructorRva = 0x721BE8C;
constexpr uintptr_t kRpcManagerLoggedInRva = 0x721B788;
constexpr uintptr_t kPlantingInitRva = 0x5EE1E44;
constexpr uintptr_t kPlantingStartRva = 0x5EE1FB8;
constexpr uintptr_t kPlantingStateUpdatedRva = 0x5EE27F4;
constexpr uintptr_t kLocationControllerAwakeRva = 0x71576D8;
constexpr uintptr_t kRequestConstructorRva = 0x71D2DE8;
constexpr uintptr_t kRequestSetMapObjectIdRva = 0x71D2FE0;
constexpr uintptr_t kRequestSetIncludeFailureRva = 0x71D30C0;
constexpr uintptr_t kSendClaimRva = 0x720AF0C;
constexpr uintptr_t kTaskIsCompletedRva = 0xC8EEDAC;
constexpr uintptr_t kTaskIsFaultedRva = 0xC8F47C0;

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
using ClassGetName = const char *(*)(void *);
using DomainGet = void *(*)();
using DomainGetAssemblies = const void **(*)(void *, size_t *);
using AssemblyGetImage = void *(*)(const void *);
using ClassFromName = void *(*)(void *, const char *, const char *);
using ClassGetMethodFromName = void *(*)(void *, const char *, int);
using ObjectNew = void *(*)(void *);
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
ClassGetName class_get_name{};
DomainGet domain_get{};
DomainGetAssemblies domain_get_assemblies{};
AssemblyGetImage assembly_get_image{};
ClassFromName class_from_name{};
ClassGetMethodFromName class_get_method_from_name{};
ObjectNew object_new{};
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
PikminTaskPreparerConstructor original_pikmin_task_preparer_constructor{};
PikminTaskActionManagerConstructor original_pikmin_task_action_manager_constructor{};
TaskBool task_is_completed{};
TaskBool task_is_faulted{};
void *request_class{};
void *rpc_manager{};
void *planting_controller{};
void *interaction_settings{};
void *location_controller{};
void *return_preparer{};
void *return_inventory_manager{};
void *return_action_manager{};
char flower_log_path[512]{};
char mode_path[512]{};
char target_path[512]{};
char status_path[512]{};
char claim_log_path[512]{};
char system_gps_path[512]{};
char return_trace_path[512]{};
char return_mode_path[512]{};
char return_postcard_policy_path[512]{};
char return_status_path[512]{};
char return_batch_limit_path[512]{};
char compatibility_path[512]{};
std::map<std::string, FlowerRecord> flowers;
std::map<std::string, std::string> last_flower_state;
long long last_tick_ms{};
long long last_claim_ms{};
long long last_status_ms{};
long long last_task_list_trace_ms{};
long long last_return_dry_run_ms{};
bool test_once_sent{};
bool target_loaded{};
bool return_one_dispatched{};
bool return_batch_waiting{};
bool return_batch_stopped{};
int return_batch_baseline_count{};
int return_batch_completed{};
long long return_batch_dispatched_ms{};
std::string last_return_batch_mode;
void *pending_task{};
uint32_t pending_task_handle{};
std::string pending_id;
double pending_gps_lat{};
double pending_gps_lng{};
double pending_distance{};
std::string last_result = "none";

long long now_ms();

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
}

void write_compatibility_status(bool compatible, off_t observed_size) {
    FILE *file = std::fopen(compatibility_path, "w");
    if (!file) return;
    std::fprintf(file, "v151\t%s\t%lld\t%lld\n", compatible ? "compatible" : "incompatible",
                 static_cast<long long>(kExpectedIl2CppSize), static_cast<long long>(observed_size));
    std::fclose(file);
    chmod(compatibility_path, 0644);
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
    if (!items || count < 0 || count > 128) {
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

// Runs on the game's main update thread.  It asks the game's own readiness
// predicate about each live inventory task and only writes diagnostics.
void dry_run_return_tasks() {
    const long long now = now_ms();
    if (!return_preparer || !return_inventory_manager || !original_get_pikmin_task_list ||
        !original_should_prepare_completion || !get_inventory_item_id ||
        now - last_return_dry_run_ms < 5000) return;
    last_return_dry_run_ms = now;
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) return;
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > 128) return;
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
        }
    }
    LOGI("[RETURN-DIAG] dry-run tasks=%d preparerReady=%d finishDue=%d",
         count, ready_count, due_count);
}

void *find_class(const char *namespc, const char *name) {
    if (!domain_get || !domain_get_assemblies || !assembly_get_image || !class_from_name) return nullptr;
    size_t count{};
    const void **assemblies = domain_get_assemblies(domain_get(), &count);
    for (size_t index = 0; assemblies && index < count; ++index) {
        void *klass = class_from_name(assembly_get_image(assemblies[index]), namespc, name);
        if (klass) return klass;
    }
    return nullptr;
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
    void *finish_method = proto_class ? class_get_method_from_name(proto_class, "get_FinishTimeMs", 0) : nullptr;
    void *finish_entry = finish_method ? *reinterpret_cast<void **>(finish_method) : nullptr;
    if (!list_entry || !proto_entry || !finish_entry) {
        LOGE("[RETURN-DIAG] task-list or task-time method not found");
        return;
    }
    get_pikmin_task_proto = reinterpret_cast<GetPikminTaskProto>(proto_entry);
    get_task_finish_time_ms = reinterpret_cast<GetTaskFinishTimeMs>(finish_entry);
    A64HookFunction(list_entry, reinterpret_cast<void *>(hooked_get_pikmin_task_list),
                    reinterpret_cast<void **>(&original_get_pikmin_task_list));
    LOGI("[RETURN-DIAG] task-list hook installed method=%p entry=%p", list_method, list_entry);
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
    // The Magisk service writes the currently active system location here.
    // It is used only when the version-specific in-memory layout is unknown.
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
    if (!domain_get || !domain_get_assemblies || !assembly_get_image || !class_from_name) return false;
    void *domain = domain_get();
    size_t count{};
    const void **assemblies = domain_get_assemblies(domain, &count);
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
    const bool online = network_available();
    const bool planting = is_planting();
    write_status(mode, online, planting);
    if (mode != "test_once" && mode != "auto") return;
    if (mode == "test_once" && test_once_sent) return;
    if (pending_task) return;
    load_test_target();
    if (!rpc_manager || !online || !planting) {
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
        if (distance > 100.0) continue;
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
    // v151 registers live flowers without consistently calling the manager's
    // Update method.  Run the same mode-gated heartbeat here so diagnostic
    // status and opt-in automation both see those observations.
    maybe_claim();
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
}
void hooked_register_map_object(void *self, void *map_object, int tag, void *method_info) {
    if (original_register_map_object) original_register_map_object(self, map_object, tag, method_info);
    log_flower(map_object);
    maybe_claim();
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
    if (return_one_dispatched || read_return_mode() != "one" || !return_action_manager ||
        !return_inventory_manager || !original_get_pikmin_task_list || !original_complete_pikmin_task ||
        !get_inventory_item_id || !get_pikmin_task_proto || !get_task_finish_time_ms) return;
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) return;
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > 128) return;
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
        append_return_trace("native-dispatch-one", return_action_manager, task_id);
        const bool discard_postcard = return_discard_postcard();
        void *async_task = original_complete_pikmin_task(return_action_manager, task_id, discard_postcard, nullptr);
        if (async_task && gchandle_new) gchandle_new(async_task, false);
        LOGI("[RETURN-DIAG] native one-shot dispatched task=%s async=%p",
             utf8_string(task_id).c_str(), async_task);
        return;
    }
}

void maybe_dispatch_return_batch() {
    const std::string mode = read_return_mode();
    if (mode != "batch") {
        // Leaving batch mode explicitly re-arms the next batch, without
        // allowing a paused process to restart itself unexpectedly.
        if (last_return_batch_mode == "batch") {
            return_batch_waiting = false;
            return_batch_stopped = false;
            return_batch_baseline_count = 0;
            return_batch_completed = 0;
        }
        last_return_batch_mode = mode;
        return;
    }
    if (last_return_batch_mode != "batch") {
        return_batch_waiting = false;
        return_batch_stopped = false;
        return_batch_baseline_count = 0;
        return_batch_completed = 0;
        last_return_batch_mode = "batch";
    }
    if (return_batch_stopped || !return_action_manager ||
        !return_inventory_manager || !original_get_pikmin_task_list || !original_complete_pikmin_task ||
        !get_inventory_item_id || !get_pikmin_task_proto || !get_task_finish_time_ms) return;
    void *list = original_get_pikmin_task_list(return_inventory_manager, nullptr);
    if (!list) return;
    void *items = *reinterpret_cast<void **>(static_cast<uint8_t *>(list) + 0x10);
    const int count = *reinterpret_cast<int *>(static_cast<uint8_t *>(list) + 0x18);
    if (!items || count < 0 || count > 128) return;
    const long long now = now_ms();
    if (return_batch_waiting) {
        if (count < return_batch_baseline_count) {
            return_batch_waiting = false;
            ++return_batch_completed;
            LOGI("[RETURN-DIAG] batch confirmed completed=%d", return_batch_completed);
            write_return_status("batch-confirmed", count, return_batch_completed, false,
                                return_discard_postcard());
        } else if (now - return_batch_dispatched_ms > 20000) {
            return_batch_stopped = true;
            LOGE("[RETURN-DIAG] batch stopped: inventory did not update within 20s");
            write_return_status("batch-stopped-timeout", count, return_batch_completed, true,
                                return_discard_postcard());
        }
        return;
    }
    const int batch_limit = return_batch_limit();
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
        append_return_trace("native-dispatch-batch", return_action_manager, task_id);
        const bool discard_postcard = return_discard_postcard();
        write_return_status("batch-dispatched", count, return_batch_completed, true, discard_postcard);
        void *async_task = original_complete_pikmin_task(return_action_manager, task_id, discard_postcard, nullptr);
        if (async_task && gchandle_new) gchandle_new(async_task, false);
        LOGI("[RETURN-DIAG] batch dispatched task=%s async=%p", utf8_string(task_id).c_str(), async_task);
        return;
    }
}
void hooked_map_update(void *self, void *method_info) {
    if (original_map_update) original_map_update(self, method_info);
    maybe_claim();
    dry_run_return_tasks();
    maybe_dispatch_one_return_task();
    maybe_dispatch_return_batch();
}
void hooked_planting_init(void *self, void *method_info) {
    if (original_planting_init) original_planting_init(self, method_info);
    planting_controller = self;
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
    object_new = reinterpret_cast<ObjectNew>(xdl_sym(handle, "il2cpp_object_new", nullptr));
    string_new = reinterpret_cast<StringNew>(xdl_sym(handle, "il2cpp_string_new", nullptr));
    gchandle_new = reinterpret_cast<GcHandleNew>(xdl_sym(handle, "il2cpp_gchandle_new", nullptr));
    object_get_class = reinterpret_cast<ObjectGetClass>(xdl_sym(handle, "il2cpp_object_get_class", nullptr));
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
    std::snprintf(return_mode_path, sizeof(return_mode_path), "%s/files/return_rpc_mode.txt", game_data_dir);
    std::snprintf(return_postcard_policy_path, sizeof(return_postcard_policy_path), "%s/files/return_postcard_policy.txt", game_data_dir);
    std::snprintf(return_status_path, sizeof(return_status_path), "%s/files/return_rpc_status.tsv", game_data_dir);
    std::snprintf(return_batch_limit_path, sizeof(return_batch_limit_path), "%s/files/return_batch_limit.txt", game_data_dir);
    std::snprintf(compatibility_path, sizeof(compatibility_path), "%s/files/compatibility_status.tsv", game_data_dir);
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
    install_return_diagnostic_hook();
    LOGI("[NECTAR] v151 RPC hooks installed base=%" PRIxPTR " mode=%s", base, mode_path);
}

}  // namespace

void hack_prepare(const char *game_data_dir, void *, size_t) {
    std::thread worker(start, game_data_dir);
    worker.detach();
}
