#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "geo.h"

namespace TC {
	namespace detail {
		struct Stop {
			std::string name;
			Coordinates point;
		};

		struct Bus {
			std::string name;
			std::vector<Stop*> stops;
			bool is_circle{ true };
			int total_stops{};
			int unique_stops{};
			uint64_t actual_distance{};
			double curveture{};
		};

		struct BusComparator {
			bool operator() (Bus* lbus, Bus* rbus) const {
				return lbus->name < rbus->name;
			}
		};

		struct StopsPtrHash {
			size_t operator() (std::pair<Stop*, Stop*> stops) const {
				return hasher(stops.first) + hasher(stops.second) * 37 * 37;
			}
			std::hash<const void*> hasher;
		};
	}

	class TransportCatalogue {
	public:
		void AddStop(detail::Stop&& stop);
		detail::Stop* FindStop(const std::string_view& stop) const;
		void AddBus(detail::Bus&& bus);
		const detail::Bus* FindBus(const std::string_view& bus_name) const;
		std::ostream& GetBusInfo(std::ostream& out, const std::string_view& bus_name) const;
		std::ostream& GetStopInfo(std::ostream& out, const std::string_view& stop_name) const;
		void AddDistance(std::pair<detail::Stop*, detail::Stop*> pair_stops, int distance_value);

	private:
		std::deque<detail::Stop> stops_;
		std::unordered_map <std::string_view, detail::Stop*> stopname_to_stop_;
		std::deque<detail::Bus> buses_;
		std::unordered_map<std::string_view, detail::Bus*> busname_to_bus_;
		std::unordered_map <std::string_view, std::set<detail::Bus*, detail::BusComparator>> stopname_to_buses_;
		std::unordered_map <std::pair<detail::Stop*, detail::Stop*>, int, detail::StopsPtrHash> stops_distance_;

		int CountBusTotalStops(const detail::Bus* bus) const;
		int CountBusUniqueStops(const detail::Bus* bus) const;
		std::pair<uint64_t, double> ComputeRouteDistance(const detail::Bus* bus) const;
		double ComputeGeoDistance(const detail::Bus* bus) const;
		uint64_t ComputeActualDistance(const detail::Bus* bus) const;
		void FillStopnameToBuses(detail::Bus* bus);
	};
}
