#include "hack.h"
#include "log.h"
#include "xdl.h"
#include "And64InlineHook.hpp"

#include <cinttypes>
#include <cmath>
#include <cstdio>
#include <cstring>
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
TaskBool task_is_completed{};
TaskBool task_is_faulted{};
void *request_class{};
void *rpc_manager{};
void *planting_controller{};
void *interaction_settings{};
void *location_controller{};
char flower_log_path[512]{};
char mode_path[512]{};
char target_path[512]{};
char status_path[512]{};
char claim_log_path[512]{};
std::map<std::string, FlowerRecord> flowers;
std::map<std::string, std::string> last_flower_state;
long long last_tick_ms{};
long long last_claim_ms{};
long long last_status_ms{};
bool test_once_sent{};
bool target_loaded{};
void *pending_task{};
uint32_t pending_task_handle{};
std::string pending_id;
double pending_gps_lat{};
double pending_gps_lng{};
double pending_distance{};
std::string last_result = "none";

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
    return false;
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
    if (!file) return;
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
}
void hooked_map_update(void *self, void *method_info) {
    if (original_map_update) original_map_update(self, method_info);
    maybe_claim();
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
    request_constructor = reinterpret_cast<RequestConstructor>(base + kRequestConstructorRva);
    request_set_map_object_id = reinterpret_cast<RequestSetString>(base + kRequestSetMapObjectIdRva);
    request_set_include_failure = reinterpret_cast<RequestSetBool>(base + kRequestSetIncludeFailureRva);
    send_claim = reinterpret_cast<SendClaim>(base + kSendClaimRva);
    struct stat il2cpp_stat{};
    if (!info.dli_fname || stat(info.dli_fname, &il2cpp_stat) != 0
            || il2cpp_stat.st_size != kExpectedIl2CppSize) {
        LOGE("[NECTAR] unsupported libil2cpp.so; hooks were not installed");
        return;
    }
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
    LOGI("[NECTAR] v151 RPC hooks installed base=%" PRIxPTR " mode=%s", base, mode_path);
}

}  // namespace

void hack_prepare(const char *game_data_dir, void *, size_t) {
    std::thread worker(start, game_data_dir);
    worker.detach();
}
