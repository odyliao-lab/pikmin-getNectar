#pragma once
#include <cmath>
#include <sstream>
#include <string>

namespace pikmin {
// Ordinary armed has no radius cap. Farm and named batch retain their old gates.
inline bool dispatch_distance_allowed(bool nearby, bool batch, double metres) {
    return std::isfinite(metres) && metres >= 0 && (nearby || metres <= (batch ? 4.0 : 200.0));
}
// Separate from the legacy farm/batch filter. Missing config = fruit + seed.
inline unsigned nearby_selection(const std::string &value) {
    if (value.size() > 128) return 0;
    std::istringstream input(value);
    std::string word;
    unsigned mask = 0;
    while (input >> word) {
        if (word == "fruit") mask |= 1;
        else if (word == "seed") mask |= 2;
        else if (word == "gift") mask |= 4;
        else return 0;
    }
    return mask;
}
inline bool nearby_kind_allowed(const std::string &legacy, unsigned mask, const std::string &kind) {
    if (legacy == "farm") return kind == "fruit" || kind == "seed";
    const unsigned bit = kind == "fruit" ? 1 : kind == "seed" ? 2 : kind == "gift" ? 4 : 0;
    return (legacy == "all" || legacy == kind) && (mask & bit) != 0;
}
}
