#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include "domain.h"
#include <map>

namespace TC {
	class TransportCatalogue {
	public:
		void AddStop(const detail::Stop&& stop);
		//достаточно имя, остановки, кольцо
		void AddBus(const detail::Bus&& bus);

		const detail::Stop* FindStop(const std::string_view stop) const;
		const detail::Bus* FindBus(const std::string_view bus_name) const;

		const std::deque<detail::Stop>& GetAllStops() const;
		const std::deque<detail::Bus>& GetAllBuses() const;
		std::set<const detail::Bus*, detail::BusComparator> GetAllBusesSorted() const;
		const std::unordered_map <std::pair<const detail::Stop*, const detail::Stop*>
			, uint32_t, detail::StopsPtrHash>& GetAllDistances() const;

		std::ostream& GetBusInfo(std::ostream& out, const std::string_view bus_name) const;
		std::ostream& GetStopInfo(std::ostream& out, const std::string_view stop_name) const;

		const std::set<detail::Bus*, detail::BusComparator>* GetBusesByStop(const std::string_view stop_name) const;

		void SetDistance(std::pair<const detail::Stop*, const detail::Stop*> pair_stops, int distance_value);
		uint32_t FindTwoStopsDistance(std::pair<const detail::Stop*, const detail::Stop*> pair_stops) const;

	private:
		std::deque<detail::Stop> stops_;
		std::deque<detail::Bus> buses_;
		std::unordered_map <std::string_view, detail::Stop*> stopname_to_stop_;
		std::map<std::string_view, detail::Bus*> busname_to_bus_;
		std::unordered_map <std::string_view, std::set<detail::Bus*, detail::BusComparator>> stopname_to_buses_;
		std::unordered_map <std::pair<const detail::Stop*, const detail::Stop*>, uint32_t, detail::StopsPtrHash> stops_distance_;

		int CountBusTotalStops(const detail::Bus* bus) const;
		int CountBusUniqueStops(const detail::Bus* bus) const;

		std::pair<uint64_t, double> ComputeRouteDistance(const detail::Bus* bus) const;
		double ComputeBusGeoDistance(const detail::Bus* bus) const;
		uint64_t ComputeBusActualDistance(const detail::Bus* bus) const;

		void FillStopnameToBuses(detail::Bus* bus);
	};
}
