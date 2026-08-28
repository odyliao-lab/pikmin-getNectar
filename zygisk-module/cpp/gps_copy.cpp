#include "gps_copy.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "And64InlineHook.hpp"
#include "log.h"

namespace {

// Adapted with permission from odyliao-lab/Pikmin-getGPS.  Pikmin Bloom 152.0
// / versionCode 1787540739.  These signatures make this
// feature fail closed on a future game build instead of guessing a UI address.
constexpr uintptr_t kRvaListItemOnClick = 0x5F8BA64;
constexpr uintptr_t kRvaGetSpawnLocation = 0x5FAEF58;
constexpr uintptr_t kListItemCurrentTaskOffset = 0xD8;
constexpr uint8_t kListItemOnClickSignature[] = {
        0xFF, 0xC3, 0x00, 0xD1, 0xFE, 0x57, 0x01, 0xA9,
        0xF4, 0x4F, 0x02, 0xA9, 0x14, 0x9A, 0x04, 0x90,
        0xF3, 0x03, 0x00, 0xAA, 0x88, 0x52, 0x7E, 0x39,
        0x08, 0x01, 0x00, 0x37, 0x80, 0x7A, 0x04, 0xD0,
        0x00, 0xA0, 0x26, 0x91, 0x21, 0x00, 0x80, 0x52,
        0x35, 0x00, 0x80, 0x52, 0x03, 0x8C, 0xD0, 0x97};
constexpr uint8_t kGetSpawnLocationSignature[] = {
        0x00, 0x40, 0x40, 0xF9, 0x40, 0x00, 0x00, 0xB4,
        0x03, 0x00, 0x00, 0x14, 0xFE, 0x0F, 0x1F, 0xF8,
        0xB5, 0x6B, 0xCE, 0x97, 0xFE, 0x0F, 0x1D, 0xF8};

struct LatLng { double lat; double lng; };
using ListItemOnClickFn = void (*)(void *, uint8_t, void *);
using GetSpawnLocationFn = LatLng (*)(void *, void *);

JavaVM *g_vm{};
ListItemOnClickFn g_original_on_click{};
GetSpawnLocationFn g_get_spawn_location{};
char g_mode_path[512]{};
char g_status_path[512]{};
char g_history_path[512]{};
std::mutex g_copy_mutex;
double g_last_lat = 999.0, g_last_lng = 999.0;
int64_t g_last_copy_ms{};

int64_t realtime_ms() { timespec now{}; clock_gettime(CLOCK_REALTIME, &now); return static_cast<int64_t>(now.tv_sec) * 1000 + now.tv_nsec / 1000000; }
int64_t monotonic_ms() { timespec now{}; clock_gettime(CLOCK_MONOTONIC, &now); return static_cast<int64_t>(now.tv_sec) * 1000 + now.tv_nsec / 1000000; }
bool valid_coordinate(const LatLng &p) { return std::isfinite(p.lat) && std::isfinite(p.lng) && p.lat >= -90 && p.lat <= 90 && p.lng >= -180 && p.lng <= 180 && (std::fabs(p.lat) > 0.0000001 || std::fabs(p.lng) > 0.0000001); }

void write_status(const char *state, const LatLng *point = nullptr) {
    FILE *file = std::fopen(g_status_path, "w");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%.7f\t%.7f\n", static_cast<long long>(realtime_ms()), state,
                 point ? point->lat : 0.0, point ? point->lng : 0.0);
    std::fclose(file); chmod(g_status_path, 0644);
}

void append_history(const char *state, const LatLng &point) {
    FILE *file = std::fopen(g_history_path, "a");
    if (!file) return;
    std::fprintf(file, "%lld\t%s\t%.7f\t%.7f\n", static_cast<long long>(realtime_ms()), state, point.lat, point.lng);
    std::fclose(file); chmod(g_history_path, 0644);
}

bool auto_copy_enabled() {
    FILE *file = std::fopen(g_mode_path, "r");
    if (!file) return false;
    char mode[16]{}; const bool enabled = std::fgets(mode, sizeof(mode), file) && std::strncmp(mode, "on", 2) == 0;
    std::fclose(file); return enabled;
}

bool clear_jni_exception(JNIEnv *env) { if (!env->ExceptionCheck()) return false; env->ExceptionClear(); return true; }

bool copy_to_clipboard_and_toast(const char *coordinates) {
    if (!g_vm) return false;
    JNIEnv *env{}; bool attached = false;
    const jint state = g_vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (state == JNI_EDETACHED) { if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK) return false; attached = true; }
    else if (state != JNI_OK || !env) return false;
    bool copied = false;
    if (env->PushLocalFrame(24) != JNI_OK) { if (attached) g_vm->DetachCurrentThread(); return false; }
    jclass activity = env->FindClass("android/app/ActivityThread");
    jmethodID current = activity ? env->GetStaticMethodID(activity, "currentApplication", "()Landroid/app/Application;") : nullptr;
    jobject app = current ? env->CallStaticObjectMethod(activity, current) : nullptr;
    if (clear_jni_exception(env) || !app) goto cleanup;
    {
    jclass context = env->FindClass("android/content/Context");
    jmethodID service = context ? env->GetMethodID(context, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;") : nullptr;
    jstring clipboard_name = env->NewStringUTF("clipboard");
    jobject clipboard = service ? env->CallObjectMethod(app, service, clipboard_name) : nullptr;
    if (clear_jni_exception(env) || !clipboard) goto cleanup;
    jclass clip_data = env->FindClass("android/content/ClipData");
    jmethodID new_plain_text = clip_data ? env->GetStaticMethodID(clip_data, "newPlainText", "(Ljava/lang/CharSequence;Ljava/lang/CharSequence;)Landroid/content/ClipData;") : nullptr;
    jobject clip = new_plain_text ? env->CallStaticObjectMethod(clip_data, new_plain_text, env->NewStringUTF("Pikmin GPS"), env->NewStringUTF(coordinates)) : nullptr;
    if (clear_jni_exception(env) || !clip) goto cleanup;
    jclass clipboard_class = env->GetObjectClass(clipboard);
    jmethodID set_primary = clipboard_class ? env->GetMethodID(clipboard_class, "setPrimaryClip", "(Landroid/content/ClipData;)V") : nullptr;
    if (!set_primary) goto cleanup;
    env->CallVoidMethod(clipboard, set_primary, clip);
    if (clear_jni_exception(env)) goto cleanup;
    copied = true;
    jclass toast = env->FindClass("android/widget/Toast");
    jmethodID make = toast ? env->GetStaticMethodID(toast, "makeText", "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;") : nullptr;
    jobject notice = make ? env->CallStaticObjectMethod(toast, make, app, env->NewStringUTF("GPS 已複製"), 0) : nullptr;
    if (!clear_jni_exception(env) && notice) { jmethodID show = env->GetMethodID(toast, "show", "()V"); if (show) env->CallVoidMethod(notice, show); clear_jni_exception(env); }
    }
cleanup:
    env->PopLocalFrame(nullptr); if (attached) g_vm->DetachCurrentThread(); return copied;
}

void capture_gps(void *expedition) {
    if (!expedition || !g_get_spawn_location) return;
    const LatLng point = g_get_spawn_location(expedition, nullptr);
    if (!valid_coordinate(point)) { write_status("invalid"); return; }
    const int64_t now = monotonic_ms();
    { std::lock_guard<std::mutex> lock(g_copy_mutex);
      if (std::fabs(point.lat - g_last_lat) < 0.00000001 && std::fabs(point.lng - g_last_lng) < 0.00000001 && now - g_last_copy_ms < 1200) return;
      g_last_lat = point.lat; g_last_lng = point.lng; g_last_copy_ms = now; }
    char coordinates[64]{};
    std::snprintf(coordinates, sizeof(coordinates), "%.7f,%.7f", point.lat, point.lng);
    const bool copied = auto_copy_enabled() && copy_to_clipboard_and_toast(coordinates);
    write_status(copied ? "copied" : "captured", &point); append_history(copied ? "copied" : "captured", point);
}

void hooked_list_item_on_click(void *self, uint8_t unit, void *method) {
    if (self) capture_gps(*reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(self) + kListItemCurrentTaskOffset));
    if (g_original_on_click) g_original_on_click(self, unit, method);
}

} // namespace

void gps_copy_prepare(uintptr_t base, const char *game_data_dir, JavaVM *vm) {
    g_vm = vm;
    std::snprintf(g_mode_path, sizeof(g_mode_path), "%s/files/gps_copy_mode.txt", game_data_dir);
    std::snprintf(g_status_path, sizeof(g_status_path), "%s/files/gps_copy_status.tsv", game_data_dir);
    std::snprintf(g_history_path, sizeof(g_history_path), "%s/files/gps_copy_history.tsv", game_data_dir);
    auto *click = reinterpret_cast<void *>(base + kRvaListItemOnClick);
    auto *spawn = reinterpret_cast<void *>(base + kRvaGetSpawnLocation);
    if (std::memcmp(click, kListItemOnClickSignature, sizeof(kListItemOnClickSignature)) != 0 || std::memcmp(spawn, kGetSpawnLocationSignature, sizeof(kGetSpawnLocationSignature)) != 0) { write_status("signature-mismatch"); LOGE("[GPS] v152 signature mismatch; hook skipped"); return; }
    g_get_spawn_location = reinterpret_cast<GetSpawnLocationFn>(spawn);
    A64HookFunction(click, reinterpret_cast<void *>(hooked_list_item_on_click), reinterpret_cast<void **>(&g_original_on_click));
    if (!g_original_on_click) { write_status("hook-failed"); LOGE("[GPS] list click hook failed"); return; }
    write_status("ready"); LOGI("[GPS] v152 expedition GPS hook installed");
}
