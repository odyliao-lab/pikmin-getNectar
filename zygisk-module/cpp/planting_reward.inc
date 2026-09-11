// v152 GotItemOverlay: only FlowerPlantingReward (13), never other rewards.
// All calls run on the game's update thread; no RPC or UI coordinate injection.
using RewardInvoke = void *(*)(const void *, void *, void **, void **);
using RewardUnbox = void *(*)(void *);
RewardInvoke reward_invoke{};
RewardUnbox reward_unbox{};
SimpleMethod original_reward_start{};
void *reward_overlay{};
decltype(gchandle_new(nullptr, false)) reward_handle{};
long long reward_seen_ms{};
long long reward_poll_ms{};

void forget_reward_overlay() {
    if (reward_handle && gchandle_free) gchandle_free(reward_handle);
    reward_handle = 0;
    reward_overlay = nullptr;
}

bool reward_authorized(long long now) {
    return read_return_mode() == "all" ||
           (planting_result_stop_requested_ms > 0 && now >= planting_result_stop_requested_ms &&
            now - planting_result_stop_requested_ms <= 60000);
}

bool reward_call(void *obj, const char *name, void **result) {
    if (!obj || !reward_invoke || !object_get_class || !class_get_method_from_name) return false;
    void *klass = object_get_class(obj);
    void *method = klass ? class_get_method_from_name(klass, name, 0) : nullptr;
    if (!method) return false;
    void *exception = nullptr;
    *result = reward_invoke(method, obj, nullptr, &exception);
    if (exception) LOGI("[PLANTING-REWARD] managed exception; leaving dialog for user");
    return exception == nullptr;
}

bool reward_bool(void *obj, const char *name, bool *value) {
    void *boxed = nullptr;
    if (!reward_call(obj, name, &boxed) || !boxed || !reward_unbox) return false;
    void *data = reward_unbox(boxed);
    if (!data) return false;
    *value = *static_cast<bool *>(data);
    return true;
}

void hooked_reward_start(void *self, void *method_info) {
    if (original_reward_start) original_reward_start(self, method_info);
    forget_reward_overlay();
    const long long now = now_ms();
    if (!self || !reward_authorized(now) || !gchandle_new || !gchandle_free) return;
    // Pinned root remains valid across frames and is released on every terminal path.
    reward_handle = gchandle_new(self, true);
    if (!reward_handle) return;
    reward_overlay = self;
    reward_seen_ms = now;
    reward_poll_ms = 0;
}

void maybe_collect_planting_reward() {
    if (!reward_overlay) return;
    const long long now = now_ms();
    if (now < reward_seen_ms || now - reward_seen_ms > 60000 || !reward_authorized(now)) {
        forget_reward_overlay();
        return;
    }
    if (now - reward_seen_ms < 1000 || now - reward_poll_ms < 500) return;
    reward_poll_ms = now;
    void *common = nullptr;
    if (!reward_call(reward_overlay, "get_CommonItemDialog", &common)) {
        forget_reward_overlay(); return;
    }
    if (!common) return; // async dialog initialization has not completed
    ScopedManagedRoot common_root(common, gchandle_new, gchandle_free);
    void *reason = nullptr;
    if (!reward_call(common, "get_ReasonCase", &reason) || !reason || !reward_unbox) {
        forget_reward_overlay(); return;
    }
    void *reason_data = reward_unbox(reason);
    if (!reason_data || *static_cast<int *>(reason_data) != 13) {
        forget_reward_overlay(); return;
    }
    void *klass = object_get_class(reward_overlay);
    void *field = klass && class_get_field ? class_get_field(klass, "collectButton") : nullptr;
    void *button = nullptr;
    if (!field || !field_get_value) { forget_reward_overlay(); return; }
    field_get_value(reward_overlay, field, &button);
    if (!button) return;
    ScopedManagedRoot button_root(button, gchandle_new, gchandle_free);
    bool interactable = false, touch = false, loading = true;
    if (!reward_bool(button, "get_Interactable", &interactable) ||
        !reward_bool(button, "WillReceiveTouch", &touch) ||
        !reward_bool(button, "get_ShowLoadingVisuals", &loading)) {
        forget_reward_overlay(); return;
    }
    if (!interactable || !touch || loading) return;
    // Consume before invocation: exceptions/uncertain outcomes never trigger a retry.
    forget_reward_overlay();
    void *ignored = nullptr;
    const bool ok = reward_call(button, "InvokeClick", &ignored);
    LOGI("[PLANTING-REWARD] reason=13 collect-button %s; closure needs observation",
         ok ? "invoked" : "failed");
}

void install_planting_reward_hook() {
    if (original_reward_start || !reward_invoke || !reward_unbox) return;
    void *klass = find_class("Niantic.Ichigo.Game.ItemGet", "GotItemOverlay");
    void *method = klass ? class_get_method_from_name(klass, "Start", 0) : nullptr;
    if (method) A64HookFunction(*reinterpret_cast<void **>(method),
        reinterpret_cast<void *>(hooked_reward_start), reinterpret_cast<void **>(&original_reward_start));
    LOGI("[PLANTING-REWARD] hook=%d", original_reward_start ? 1 : 0);
}
