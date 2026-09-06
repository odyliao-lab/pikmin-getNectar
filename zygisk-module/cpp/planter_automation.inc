// Included inside nectar_rpc.cpp's anonymous namespace, after shared helpers.
// No additional hooks, gestures, growth edits, or consumable-item RPCs.
namespace planter_engine {
using Invoke = void *(*)(void *, void *, void **, void **);
using Unbox = void *(*)(void *);
using Nested = void *(*)(void *, void **);
using Param = void *(*)(void *, uint32_t);
using FromType = void *(*)(void *);
Invoke invoke{};
Unbox unbox{};
Nested nested{};
Param param{};
FromType from_type{};
char snapshot_path[512]{}, state_path[512]{}, history_path[512]{}, pending_path[512]{};
constexpr const char *control_path = "/data/local/tmp/pikmin-planter-mode.txt";
constexpr const char *steps_path = "/data/local/tmp/pikmin-planter-steps.txt";
unsigned selected_steps() {
    FILE *file = std::fopen(steps_path, "r");
    if (!file) return errno == ENOENT ? pikmin::planter::default_steps : 0;
    char buffer[130]{};
    const size_t length = std::fread(buffer, 1, sizeof(buffer), file);
    const bool failed = std::ferror(file) != 0;
    std::fclose(file);
    return failed || length > 128 ? 0 : pikmin::planter::parse_steps(std::string(buffer, length));
}
bool ok{}, initialized{}, compatible{}, restart_blocked{};
void *seed_list_method{}, *ready_method{}, *pull_method{}, *plant_method{};
void *pull_class{}, *plant_class{}, *option_class{};
long long last_tick{}, next_action{};
std::string blocked, last_event;

struct Seed {
    std::string id, label;
    int steps{}, required{};
    int64_t planted{};
    bool ready{}, starred{}, special{}, point{};
};
struct Slot { int index{}, type{}, remaining{}; std::string seed; };
struct Snapshot {
    std::map<std::string, Seed> seeds;
    std::set<std::string> pikmins;
    std::vector<Slot> slots;
    int max_slots{}, capacity{-1};
    bool complete{};
};
struct Pending {
    void *task{};
    GcHandle root{};
    long long sent{};
    std::string action, seed, pikmin;
    std::set<std::string> pikmins_before;
    int slot{-1};
    bool responded{};
} pending;

std::string mode() {
    std::ifstream file(control_path);
    std::string value; file >> value;
    return value == "pluck" || value == "auto" ? value : "off";
}
void *method(void *object, const char *name, int args = 0) {
    if (!object) { ok = false; return nullptr; }
    void *m = class_get_method_from_name(object_get_class(object), name, args);
    if (!m) ok = false;
    return m;
}
void *call(void *m, void *object, void **args = nullptr) {
    if (!ok || !m) { ok = false; return nullptr; }
    void *exception{};
    void *value = invoke(m, object, args, &exception);
    if (exception) {
        ok = false;
        LOGE("[PLANTER] managed call failed; no further action in this cycle");
    }
    return value;
}
void *get(void *object, const char *name) { return call(method(object, name), object); }
template<typename T> T scalar(void *object, const char *name) {
    void *boxed = get(object, name);
    if (!boxed) { ok = false; return {}; }
    void *value = unbox(boxed);
    if (!value) { ok = false; return {}; }
    return *static_cast<T *>(value);
}
void *at(void *list, int index) {
    void *args[]{&index};
    return call(method(list, "get_Item", 1), list, args);
}
int size(void *list, int max = 10000) {
    int count = scalar<int>(list, "get_Count");
    if (count < 0 || count > max) { ok = false; return 0; }
    return count;
}
std::string str(void *object, const char *name) { return read_string(get(object, name)); }
void *create(void *klass) {
    if (!klass || !ok) { ok = false; return nullptr; }
    void *object = object_new(klass);
    ScopedManagedRoot root(object, gchandle_new, gchandle_free);
    call(class_get_method_from_name(klass, ".ctor", 0), object);
    return object;
}
bool atomic_file(const char *path, const std::string &content) {
    std::string tmp = std::string(path) + ".tmp";
    FILE *file = std::fopen(tmp.c_str(), "w");
    if (!file) return false;
    bool written = std::fwrite(content.data(), 1, content.size(), file) == content.size();
    bool flushed = std::fflush(file) == 0 && fsync(fileno(file)) == 0;
    bool closed = std::fclose(file) == 0;
    if (!written || !flushed || !closed) return false;
    chmod(tmp.c_str(), 0644);
    return std::rename(tmp.c_str(), path) == 0;
}
void status(const std::string &event, const std::string &detail = "") {
    atomic_file(state_path, "v1\t" + std::to_string(now_ms()) + "\t" +
            std::to_string(getpid()) + "\t" + event + "\t" + mode() + "\t" + detail + "\n");
}
void history(const std::string &event, const std::string &id, int slot, const std::string &detail = "") {
    std::ifstream previous(history_path);
    std::vector<std::string> rows;
    std::string row;
    while (std::getline(previous, row))
        if (std::strtoll(row.c_str(), nullptr, 10) >= now_ms() - 86400000LL) rows.push_back(row);
    std::string content;
    const size_t start = rows.size() > 499 ? rows.size() - 499 : 0;
    for (size_t i = start; i < rows.size(); ++i) content += rows[i] + "\n";
    content += std::to_string(now_ms()) + "\t" + event + "\t" + id + "\t" +
            std::to_string(slot) + "\t" + detail + "\n";
    atomic_file(history_path, content);
    LOGI("[PLANTER] %s seed=%s slot=%d detail=%s", event.c_str(), id.c_str(), slot, detail.c_str());
}
void init() {
    if (initialized) return;
    initialized = true;
    if (!invoke || !unbox || !nested || !param || !from_type || !gchandle_new || !gchandle_free) return;
    void *inventory = find_class("Niantic.Ichigo.Inventory", "InventoryManager");
    seed_list_method = verified_v152_method(inventory, "GetPikminSeedList", 0, 0x7298528);
    ready_method = verified_v152_method(find_class("Niantic.Ichigo.Utils", "PikminSeedProtoExtensions"),
            "IsReadyToPluck", 1, 0x6CDE6B8);
    void *rpc = find_class("Niantic.Ichigo.Rpc", "RpcManager");
    pull_method = verified_v152_method(rpc, "SendPullPikminRpcAsync", 3, 0x7259FE4);
    plant_method = verified_v152_method(rpc, "SendSetPikminSeedRpcAsync", 3, 0x725BB2C);
    pull_class = find_class("Ichigo.Proto", "PullPikminRequestProto");
    plant_class = find_class("Ichigo.Proto", "SetPikminSeedRequestProto");
    void *iter{};
    for (void *child = plant_class ? nested(plant_class, &iter) : nullptr; child; child = nested(plant_class, &iter)) {
        if (std::strcmp(class_get_name(child), "Types") != 0) continue;
        void *sub_iter{};
        for (void *sub = nested(child, &sub_iter); sub; sub = nested(child, &sub_iter))
            if (std::strcmp(class_get_name(sub), "SlotOptionProto") == 0) option_class = sub;
    }
    compatible = seed_list_method && ready_method && pull_method && plant_method &&
            pull_class && plant_class && option_class;
    restart_blocked = access(pending_path, F_OK) == 0;
    LOGI("[PLANTER] metadata compatible=%d restartPending=%d seedList=%p ready=%p pull=%p plant=%p option=%p",
         compatible, restart_blocked, seed_list_method, ready_method, pull_method, plant_method, option_class);
}
Snapshot capture() {
    Snapshot s;
    ok = true;
    void *list = call(seed_list_method, return_inventory_manager);
    ScopedManagedRoot root(list, gchandle_new, gchandle_free);
    int count = size(list);
    for (int i = 0; i < count && ok; ++i) {
        void *item = at(list, i);
        Seed seed;
        seed.id = str(item, "get_Id");
        void *proto = get(item, "get_Proto");
        seed.required = scalar<int>(proto, "get_RequiredSteps");
        seed.steps = scalar<int>(proto, "get_CurrentSteps");
        seed.planted = scalar<int64_t>(proto, "get_PlantedTimeMs");
        seed.starred = scalar<bool>(proto, "get_Starred");
        seed.special = scalar<bool>(proto, "get_DisableConsecutivePluck");
        seed.point = get(proto, "get_BirthPlacePoint") != nullptr;
        void *args[]{proto};
        void *boxed = call(ready_method, nullptr, args);
        if (!boxed) { ok = false; break; }
        seed.ready = pikmin::planter::mature(seed.planted, seed.steps, seed.required,
                *static_cast<bool *>(unbox(boxed)));
        seed.label = describe_seed(proto);
        if (seed.id.empty() || s.seeds.count(seed.id)) { ok = false; break; }
        s.seeds.emplace(seed.id, seed);
    }
    void *planter = get(return_inventory_manager, "GetPlanter");
    void *proto = get(planter, "get_Proto");
    ScopedManagedRoot planter_root(proto, gchandle_new, gchandle_free);
    s.max_slots = scalar<int>(proto, "get_NumMaxSlots");
    void *slots = get(proto, "get_Slot");
    int slot_count = size(slots, 8);
    std::set<int> indices;
    std::set<std::string> planted_ids;
    for (int i = 0; i < slot_count && ok; ++i) {
        void *slot = at(slots, i);
        Slot v;
        v.index = scalar<int>(slot, "get_Index");
        v.type = scalar<int>(slot, "get_SlotType");
        v.remaining = scalar<int>(slot, "get_RemainingUse");
        v.seed = str(slot, "get_PikminSeedId");
        if (!indices.insert(v.index).second || (!v.seed.empty() &&
                (!s.seeds.count(v.seed) || !planted_ids.insert(v.seed).second))) ok = false;
        s.slots.push_back(v);
    }
    void *pikmins = get(return_inventory_manager, "GetPikminList");
    ScopedManagedRoot pikmin_root(pikmins, gchandle_new, gchandle_free);
    int pikmin_count = size(pikmins);
    for (int i = 0; i < pikmin_count && ok; ++i) {
        std::string id = str(at(pikmins, i), "get_Id");
        if (id.empty() || !s.pikmins.insert(id).second) ok = false;
    }
    void *capacities = get(return_inventory_manager, "GetItemCapacityList");
    ScopedManagedRoot capacity_root(capacities, gchandle_new, gchandle_free);
    int capacity_count = size(capacities, 16);
    if (capacity_count == 1) {
        void *capacity = get(get(at(capacities, 0), "get_Proto"), "get_Pikmin");
        if (capacity) s.capacity = scalar<int>(capacity, "get_CurrentCount");
    }
    s.complete = ok && slot_count > 0 && s.max_slots > 0 && s.max_slots <= 8;
    return s;
}
void write_snapshot(const Snapshot &s) {
    if (!s.complete) return; // Never overwrite a complete snapshot with a partial one.
    std::string out = "v1\t" + std::to_string(now_ms()) + "\t" + std::to_string(getpid()) +
            "\t" + std::to_string(s.pikmins.size()) + "\t" + std::to_string(s.capacity) + "\n";
    const unsigned steps = selected_steps();
    out += "steps\t" + std::to_string(steps) + "\n";
    for (const Slot &slot : s.slots) {
        auto found = s.seeds.find(slot.seed);
        Seed v = found == s.seeds.end() ? Seed{} : found->second;
        out += "slot\t" + std::to_string(slot.index) + "\t" + std::to_string(slot.type) + "\t" +
                std::to_string(slot.remaining) + "\t" + slot.seed + "\t" + std::to_string(v.steps) + "\t" +
                std::to_string(v.required) + "\t" + (v.ready ? "ready" : "growing") + "\t" + v.label + "\n";
    }
    for (const auto &[id, v] : s.seeds)
        out += "seed\t" + id + "\t" + std::to_string(v.planted) + "\t" + std::to_string(v.required) +
                "\t" + (v.starred ? "1" : "0") + "\t" + (v.special ? "1" : "0") + "\t" + v.label +
                "\t" + (pikmin::planter::step_allowed(steps, v.required) ? "1" : "0") + "\n";
    out += "end\t" + std::to_string(s.seeds.size()) + "\n";
    atomic_file(snapshot_path, out);
}
void finish(const std::string &event, bool confirmed) {
    history(event, pending.seed, pending.slot, pending.pikmin);
    status(event, pending.seed);
    if (pending.root) gchandle_free(pending.root);
    pending = {};
    // A definitive response is terminal. Uncertain results retain the ledger.
    if (confirmed) std::remove(pending_path);
    else blocked = event;
    next_action = now_ms() + 5000;
}
void observe_pending(const Snapshot &s) {
    ok = true;
    if (!pending.responded) {
        if (!scalar<bool>(pending.task, "get_IsCompleted") || !ok) {
            if (now_ms() - pending.sent > 30000) status("waiting-rpc-no-retry", pending.seed);
            return;
        }
        if (scalar<bool>(pending.task, "get_IsFaulted") || scalar<bool>(pending.task, "get_IsCanceled") || !ok) {
            finish("rpc-failed-review-required", false); return;
        }
        void *response = get(pending.task, "get_Result");
        ScopedManagedRoot response_root(response, gchandle_new, gchandle_free);
        int result = scalar<int>(get(response, "get_Status"), "get_Status");
        if (!ok || result != 1) { finish("server-rejected-review-required", false); return; }
        if (pending.action == "pluck") {
            void *ids = get(response, "get_Pikmin");
            if (size(ids, 16) != 1 || !ok) { finish("unexpected-response-review-required", false); return; }
            pending.pikmin = read_string(at(ids, 0));
            if (pending.pikmin.empty() || pending.pikmins_before.count(pending.pikmin) || !ok) {
                finish("missing-new-pikmin-review-required", false); return;
            }
        }
        pending.responded = true;
        history("server-success", pending.seed, pending.slot, pending.pikmin);
    }
    if (!s.complete) return;
    bool matched_slot = false;
    for (const auto &slot : s.slots) if (slot.index == pending.slot)
        matched_slot = pending.action == "pluck" ? slot.seed != pending.seed : slot.seed == pending.seed;
    auto found = s.seeds.find(pending.seed);
    bool confirmed = pending.action == "pluck"
            ? found == s.seeds.end() && s.pikmins.count(pending.pikmin) && matched_slot
            : found != s.seeds.end() && found->second.planted > 0 && matched_slot;
    if (confirmed) finish(pending.action + "-confirmed", true);
    else status("waiting-inventory-no-retry", pending.seed);
}
void start(const std::string &action, const Seed &seed, const Slot &slot, const Snapshot &s) {
    ok = true;
    void *request = create(action == "pluck" ? pull_class : plant_class);
    ScopedManagedRoot request_root(request, gchandle_new, gchandle_free);
    void *id = string_new(seed.id.c_str());
    ScopedManagedRoot id_root(id, gchandle_new, gchandle_free);
    if (action == "pluck") {
        void *list = get(request, "get_SeedId");
        void *iter{}, *add{};
        void *klass = list ? object_get_class(list) : nullptr;
        void *strings = find_class("System", "String");
        for (void *m = klass ? class_get_methods(klass, &iter) : nullptr; m; m = class_get_methods(klass, &iter))
            if (std::strcmp(method_get_name(m), "Add") == 0 && method_get_param_count(m) == 1 &&
                    from_type(param(m, 0)) == strings) { add = m; break; }
        void *args[]{id};
        call(add, list, args);
        if (size(list, 1) != 1) ok = false;
    } else {
        void *args[]{id};
        call(method(request, "set_SeedId", 1), request, args);
        void *item = call(method(return_inventory_manager, "GetPikminSeed", 1), return_inventory_manager, args);
        void *point = get(get(item, "get_Proto"), "get_BirthPlacePoint");
        if (!point) ok = false;
        // Matches PelletPlanter.SendSetPikminSeedRpcAsync: original seed birthplace.
        void *point_args[]{point};
        call(method(request, "set_Point", 1), request, point_args);
        void *option = create(option_class);
        ScopedManagedRoot option_root(option, gchandle_new, gchandle_free);
        int index = slot.index;
        void *index_args[]{&index};
        call(method(option, "set_SlotIndex", 1), option, index_args);
        void *option_args[]{option};
        call(method(request, "set_SlotOption", 1), request, option_args);
    }
    if (!ok) { blocked = "request-build-failed"; status(blocked); return; }
    const long long sent = now_ms();
    if (!atomic_file(pending_path, std::to_string(sent) + "\t" + action + "\t" + seed.id +
            "\t" + std::to_string(slot.index) + "\n")) {
        blocked = "audit-write-failed"; status(blocked); return;
    }
    history(action + "-sent", seed.id, slot.index, seed.label);
    void *cancel{};
    int retry = 0; // RpcRetryPolicy.ThrowException: never blindly resend a mutation.
    void *args[]{request, &cancel, &retry};
    void *task = call(action == "pluck" ? pull_method : plant_method, rpc_manager, args);
    if (!ok || !task) { blocked = "rpc-start-uncertain-review-required"; status(blocked); return; }
    pending.task = task;
    pending.root = gchandle_new(task, false);
    pending.sent = sent; pending.action = action; pending.seed = seed.id; pending.slot = slot.index;
    pending.pikmins_before = s.pikmins;
    if (!pending.root) { blocked = "task-root-failed"; pending.task = nullptr; status(blocked); return; }
    status(action + "-sent", seed.id);
}
void tick() {
    if (!runtime_metadata_ready || !return_inventory_manager || !rpc_manager || now_ms() - last_tick < 5000) return;
    last_tick = now_ms();
    init();
    if (!compatible) { status("metadata-unavailable"); return; }
    Snapshot s = capture();
    write_snapshot(s);
    if (restart_blocked) { status("restart-pending-review-required"); return; }
    if (!blocked.empty()) { status(blocked); return; }
    if (pending.task) { observe_pending(s); return; }
    if (!s.complete) { status("inventory-incomplete"); return; }
    const std::string current_mode = mode();
    if (current_mode == "off") { status("off"); return; }
    if (now_ms() < next_action) return;
    if (!pikmin::planter::storage_room(static_cast<int>(s.pikmins.size()), s.capacity)) {
        status("storage-full-or-unknown"); return;
    }
    for (const Slot &slot : s.slots) {
        if (!pikmin::planter::permanent_slot(slot.type, slot.index, s.max_slots)) continue;
        auto found = s.seeds.find(slot.seed);
        if (found != s.seeds.end() && found->second.ready && !found->second.special) {
            start("pluck", found->second, slot, s); return;
        }
    }
    if (current_mode == "auto") {
        const unsigned steps = selected_steps();
        if (!steps) { status("no-seed-types-selected"); return; }
        std::set<std::string> in_slots;
        for (const auto &slot : s.slots) if (!slot.seed.empty()) in_slots.insert(slot.seed);
        for (const Slot &slot : s.slots) {
            if (!slot.seed.empty() || !pikmin::planter::permanent_slot(slot.type, slot.index, s.max_slots)) continue;
            const Seed *best{};
            for (const auto &[id, seed] : s.seeds)
                if (pikmin::planter::step_allowed(steps, seed.required) &&
                        pikmin::planter::can_plant(seed.planted, in_slots.count(id), seed.starred,
                        seed.required, seed.special, seed.point) &&
                        (!best || seed.required < best->required || (seed.required == best->required && id < best->id))) best = &seed;
            if (best) { start("plant", *best, slot, s); return; }
        }
    }
    status("waiting-growth-or-seed");
}
void configure(void *handle, const char *game_data_dir) {
    invoke = reinterpret_cast<Invoke>(xdl_sym(handle, "il2cpp_runtime_invoke", nullptr));
    unbox = reinterpret_cast<Unbox>(xdl_sym(handle, "il2cpp_object_unbox", nullptr));
    nested = reinterpret_cast<Nested>(xdl_sym(handle, "il2cpp_class_get_nested_types", nullptr));
    param = reinterpret_cast<Param>(xdl_sym(handle, "il2cpp_method_get_param", nullptr));
    from_type = reinterpret_cast<FromType>(xdl_sym(handle, "il2cpp_class_from_type", nullptr));
    std::snprintf(snapshot_path, sizeof(snapshot_path), "%s/files/planter_inventory.tsv", game_data_dir);
    std::snprintf(state_path, sizeof(state_path), "%s/files/planter_status.tsv", game_data_dir);
    std::snprintf(history_path, sizeof(history_path), "%s/files/planter_history.tsv", game_data_dir);
    std::snprintf(pending_path, sizeof(pending_path), "%s/files/planter_pending.tsv", game_data_dir);
}
} // namespace planter_engine
