#pragma once

namespace geo {

constexpr double EPSILON {1e-6};

struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
    bool operator<(const Coordinates& other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
