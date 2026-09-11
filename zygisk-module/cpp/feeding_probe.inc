// v152 opt-in, read-only catalogue probe. No feeding or picking RPC is resolved.
// Included after planter_automation.inc for its atomic-file writer only.
namespace feeding_probe {
using Invoke = void *(*)(void *, void *, void **, void **);
using Unbox = void *(*)(void *);
Invoke invoke{};
Unbox unbox{};
void *list_method{}, *proto_method{}, *count_method{}, *color_method{}, *kind_method{};
bool attempted{}, compatible{}, ok{};
long long last_tick{};
char path[512]{};

void *call(void *method, void *object, void **args = nullptr) {
    if (!ok || !method || !object) { ok = false; return nullptr; }
    void *exception{};
    void *value = invoke(method, object, args, &exception);
    if (exception) ok = false;
    return ok ? value : nullptr;
}
void *get(void *object, const char *name, int argc = 0, void **args = nullptr) {
    if (!object || !ok) { ok = false; return nullptr; }
    return call(class_get_method_from_name(object_get_class(object), name, argc), object, args);
}
int integer(void *boxed) {
    if (!boxed || !ok) { ok = false; return -1; }
    void *value = unbox(boxed);
    if (!value) { ok = false; return -1; }
    return *static_cast<int *>(value);
}
void publish(const char *state, const std::string &rows = "", int count = 0) {
    planter_engine::atomic_file(path, "v1\t" + std::to_string(now_ms()) + "\t" +
        std::to_string(getpid()) + "\tobserve-only\t" + state + "\t-\t" +
        std::to_string(count) + "\n" + rows + "end\n");
}
void tick() {
    const auto current = now_ms();
    if (current - last_tick < 5000) return;
    last_tick = current;
    // Separate diagnostic permission. Merely installing or enabling the UI does not invoke getters.
    std::ifstream control("/data/local/tmp/pikmin-feeding-probe.txt");
    std::string mode, extra; control >> mode;
    if (mode != "observe" || (control >> extra)) return;
    if (!invoke || !unbox || !object_get_class || !gchandle_new || !gchandle_free || !return_inventory_manager) {
        publish("waiting-inventory"); return;
    }
    if (!attempted) {
        attempted = true;
        list_method = verified_v152_method(find_class("Niantic.Ichigo.Inventory", "InventoryManager"), "GetHoneyBallList", 0, 0x7297C4C);
        proto_method = verified_v152_method(find_class("Niantic.Ichigo.Inventory", "HoneyBallInventoryItem"), "get_Proto", 0, 0x7281428);
        void *proto = find_class("Ichigo.Proto", "HoneyBallProto");
        count_method = verified_v152_method(proto, "get_NumBalls", 0, 0x5F753A0);
        color_method = verified_v152_method(proto, "get_HoneyType", 0, 0x5F753B0);
        kind_method = verified_v152_method(proto, "get_HoneyFlowerKind", 0, 0x5F753C0);
        compatible = list_method && proto_method && count_method && color_method && kind_method;
    }
    if (!compatible) { publish("method-mismatch"); return; }
    ok = true;
    void *list = call(list_method, return_inventory_manager);
    ScopedManagedRoot list_root(list, gchandle_new, gchandle_free);
    int count = integer(get(list, "get_Count"));
    if (!ok || count < 0 || count > 512) { publish("invalid-inventory"); return; }
    std::string rows;
    std::set<std::string> ids;
    int total = 0;
    for (int i = 0; i < count && ok; ++i) {
        void *args[]{&i};
        void *item = get(list, "get_Item", 1, args);
        ScopedManagedRoot item_root(item, gchandle_new, gchandle_free);
        void *proto = call(proto_method, item);
        ScopedManagedRoot proto_root(proto, gchandle_new, gchandle_free);
        int stock = integer(call(count_method, proto));
        int color = integer(call(color_method, proto));
        int kind = integer(call(kind_method, proto));
        if (!ok || stock < 0 || stock > 1000000 || kind < 0) { ok = false; break; }
        // HAPPY/special nectar and unknown types are deliberately excluded, not treated as ordinary.
        if (color < 1 || color > 4 || kind == 0) continue;
        std::string family = "flower-" + std::to_string(kind);
        std::string id = family + ":" + std::to_string(color);
        if (!ids.insert(id).second) { ok = false; break; }
        const char *colors[]{"other", "white", "red", "blue", "yellow"};
        // IDs are catalogue keys, NOT RPC ItemId. No localized name or capacity is guessed.
        rows += "kind\t" + id + "\t" + family + "\t" +
            (kind == 5 ? std::string("普通精華") : std::string("花種 #") + std::to_string(kind)) +
            "\t" + colors[color] + "\t" + (kind == 5 ? "1" : "0") + "\t" +
            std::to_string(stock) + "\t-1\t-1\n";
        ++total;
    }
    if (!ok) publish("invalid-inventory");
    else publish("catalogue-unverified", rows, total);
}
void configure(void *handle, const char *game_data_dir) {
    invoke = reinterpret_cast<Invoke>(xdl_sym(handle, "il2cpp_runtime_invoke", nullptr));
    unbox = reinterpret_cast<Unbox>(xdl_sym(handle, "il2cpp_object_unbox", nullptr));
    std::snprintf(path, sizeof(path), "%s/files/feeding_snapshot.tsv", game_data_dir);
}
} // namespace feeding_probe
