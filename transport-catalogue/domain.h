#pragma once
#include <string>
#include <vector>

#include "geo.h"

namespace TC {
	namespace detail {
		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		struct StopComparator {
			bool operator() (const Stop* lstop, const Stop* rstop) const;
		};

		struct BusStat {
			int total_stops{};
			int unique_stops{};
			uint64_t actual_distance{};
			double curveture{};
		};

		struct Bus {
			std::string name;
			std::vector<const Stop*> stops;
			bool is_circle{ true };
			BusStat statistic{};
		};

		struct BusComparator {
			bool operator() (const Bus* lbus, const Bus* rbus) const;
		};

		struct BusPtrHash {
			size_t operator() (Bus* bus) const;
			std::hash<const void*> hasher;
			std::hash<std::string> str_hasher;
		};

		struct StopsPtrHash {
			size_t operator() (std::pair<const Stop*, const Stop*> stops) const;
			std::hash<const void*> hasher;
		};
	}
}