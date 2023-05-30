#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo {

bool Coordinates::operator==(const Coordinates& other) const {
    return (std::fabs(lat - other.lat) < EPSILON) && (std::fabs(lng - other.lng) < EPSILON);
}
bool Coordinates::operator!=(const Coordinates& other) const {
    return !(*this == other);
}

bool Coordinates::operator<(const Coordinates& other) const
{
    return this->lat < other.lat;
}

double ComputeDistance(Coordinates from, Coordinates to) {
    using namespace std;
    if (from == to) {
        return 0;
    }
    static constexpr int earth_radius{ 6371000 };
    static constexpr double dr = M_PI / 180.;
    return acos(sin(from.lat * dr) * sin(to.lat * dr)
                + cos(from.lat * dr) * cos(to.lat * dr) * cos(std::abs(from.lng - to.lng) * dr))
        * earth_radius;
}

}  // namespace geo
