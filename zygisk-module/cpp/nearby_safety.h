#pragma once
#include <cmath>
#include <cstdint>

namespace pikmin {
inline bool nearby_duration_safe(int64_t ms) { return ms > 0 && ms <= 120000; }
struct NearbyFix {
    bool valid{};
    double raw_lat{}, raw_lng{}, game_lat{}, game_lng{};
    int64_t raw_wall_ms{}; // Game field name; CLOCK_BOOTTIME milliseconds, not Unix time.
};
inline bool valid_point(double lat, double lng) {
    return std::isfinite(lat) && std::isfinite(lng) && std::abs(lat) <= 90 && std::abs(lng) <= 180;
}
inline double nearby_metres(double a, double b, double c, double d) {
    constexpr double rad = 3.14159265358979323846 / 180;
    const double x = std::sin((c-a)*rad/2), y = std::sin((d-b)*rad/2);
    const double h = x*x + std::cos(a*rad)*std::cos(c*rad)*y*y;
    return 12742000 * std::asin(std::sqrt(std::fmin(1.0, std::fmax(0.0, h))));
}
/** Only processed game coordinates matched to fresh raw game fixes can open this gate.
 * One epoch invalidates ALL unsent selections on a jump, mismatch, stale input or clock rollback.
 * Re-reading one fix never counts as several fresh location updates. */
class NearbyLocationGate {
    bool anchored_{};
    double anchor_lat_{}, anchor_lng_{};
    int64_t since_{}, last_seen_{}, last_fix_{}, last_elapsed_{};
    unsigned samples_{};
    uint64_t epoch_{};
    const char *reason_{"waiting-game-location"};
public:
    void reset(const char *reason) {
        anchored_ = false; samples_ = 0; since_ = last_fix_ = 0;
        reason_ = reason; ++epoch_;
    }
    // elapsed must share the game's boot-time clock (includes device suspend).
    // steady is monotonic time for cross-tick selection/heartbeat intervals.
    void observe(const NearbyFix &f, int64_t elapsed, int64_t steady) {
        if ((last_seen_ && (steady < last_seen_ || steady-last_seen_ > 5000)) ||
                (last_elapsed_ && elapsed < last_elapsed_)) reset("location-gap");
        last_seen_ = steady; last_elapsed_ = elapsed;
        if (!f.valid || !valid_point(f.raw_lat, f.raw_lng) || !valid_point(f.game_lat, f.game_lng)) {
            reset("waiting-game-location"); return;
        }
        if (elapsed <= 0 || f.raw_wall_ms <= 0 || f.raw_wall_ms > elapsed || elapsed-f.raw_wall_ms > 5000) {
            reset("stale-game-location"); return;
        }
        if (nearby_metres(f.raw_lat, f.raw_lng, f.game_lat, f.game_lng) > 8.0) {
            reset("game-location-catching-up"); return;
        }
        if (anchored_ && (f.raw_wall_ms < last_fix_ ||
                nearby_metres(anchor_lat_, anchor_lng_, f.game_lat, f.game_lng) > 8.0))
            reset("location-moving");
        if (!anchored_) {
            anchored_ = true; anchor_lat_ = f.game_lat; anchor_lng_ = f.game_lng;
            since_ = steady; samples_ = 1; last_fix_ = f.raw_wall_ms;
        } else if (f.raw_wall_ms > last_fix_) {
            last_fix_ = f.raw_wall_ms; if (samples_ < 3) ++samples_;
        }
        reason_ = samples_ >= 3 && steady-since_ >= 3000 ? "ready" : "location-settling";
    }
    bool ready(int64_t steady) const {
        return anchored_ && samples_ >= 3 && steady >= since_ && steady-since_ >= 3000
                && steady >= last_seen_ && steady-last_seen_ <= 2000;
    }
    uint64_t epoch() const { return epoch_; }
    unsigned samples() const { return samples_; }
    const char *reason() const { return reason_; }
};
}
