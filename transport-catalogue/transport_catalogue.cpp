#include "transport_catalogue.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

using namespace std::string_literals;
namespace TC {
	void TransportCatalogue::AddStop(detail::Stop&& stop) {
		stops_.push_back(std::move(stop));
		stopname_to_stop_[stops_.back().name] = &stops_.back();
		stopname_to_buses_[stops_.back().name];
	}

	detail::Stop* TransportCatalogue::FindStop(const std::string_view& stop) const {
		return stopname_to_stop_.at(stop);
	}

	void TransportCatalogue::AddBus(detail::Bus&& bus) {
		buses_.emplace_back(std::move(bus));
		buses_.back().total_stops = CountBusTotalStops(&buses_.back());
		buses_.back().unique_stops = CountBusUniqueStops(&buses_.back());
		{
			auto distance = ComputeRouteDistance(&buses_.back());
			buses_.back().actual_distance = std::move(distance.first);
			buses_.back().curveture = std::move(distance.second);
		}
		busname_to_bus_[buses_.back().name] = &buses_.back();
		FillStopnameToBuses(&buses_.back());
	}

	const detail::Bus* TransportCatalogue::FindBus(const std::string_view& bus_name) const {
		if (busname_to_bus_.count(bus_name)) {
			return busname_to_bus_.at(bus_name);
		}
		return nullptr;
	}

	std::ostream& TransportCatalogue::GetBusInfo(std::ostream& out, const std::string_view& bus_name) const {
		const detail::Bus* bus_ptr = FindBus(bus_name);
		out << "Bus "s << bus_name;
		if (bus_ptr != nullptr) {
			out << ": "s
				<< bus_ptr->total_stops << " stops on route, "s
				<< bus_ptr->unique_stops << " unique stops, "s << std::setprecision(6)
				<< bus_ptr->actual_distance << " route length, "s
				<< bus_ptr->curveture << " curvature"s << std::endl;
		}
		else {
			out << ": not found"s << std::endl;
		}
		return out;
	}

	std::ostream& TransportCatalogue::GetStopInfo(std::ostream& out, const std::string_view& stop_name) const {
		out << "Stop "s << stop_name;
		if (stopname_to_buses_.count(stop_name)) {
			if (stopname_to_buses_.at(stop_name).size()) {
				out << ": buses"s;
				for (detail::Bus* bus : stopname_to_buses_.at(stop_name)) {
					out << ' ' << bus->name;
				}
			}
			else {
				out << ": no buses"s;
			}
		}
		else {
			out << ": not found"s;
		}
		out << std::endl;
		return out;
	}

	void TransportCatalogue::AddDistance(std::pair<detail::Stop*, detail::Stop*> pair_stops, int distance_value) {

		stops_distance_.insert({ pair_stops, distance_value });
	}

	int TransportCatalogue::CountBusTotalStops(const detail::Bus* bus) const {
		int result;
		if (bus->is_circle) {
			result = static_cast<int>(bus->stops.size());
		}
		else {
			result = static_cast<int>(bus->stops.size()) * 2 - 1;
		}
		return result;
	}

	int TransportCatalogue::CountBusUniqueStops(const detail::Bus* bus) const {
		std::vector<detail::Stop*> stops;
		auto lambda = [&stops](detail::Stop* stop) {
			if (std::find(stops.begin(), stops.end(), stop) == stops.end()) {
				stops.push_back(stop);
			}
		};
		std::for_each(bus->stops.begin(), bus->stops.end(), lambda);
		return static_cast<int>(stops.size());
	}

	std::pair<uint64_t, double> TransportCatalogue::ComputeRouteDistance(const detail::Bus* bus) const {

		uint64_t actual_distance{ ComputeActualDistance(bus) };
		double curveture{ static_cast<double>(actual_distance) / ComputeGeoDistance(bus) };
		return { actual_distance, curveture };
	}

	double TransportCatalogue::ComputeGeoDistance(const detail::Bus* bus) const {
		double geo_distance{};
		for (size_t i{ 1 }; i < bus->stops.size(); ++i) {
			geo_distance += ComputeDistance(bus->stops[i - 1]->point, bus->stops[i]->point);
		}
		if (!bus->is_circle) {
			geo_distance *= 2;
		}
		return geo_distance;
	}

	uint64_t TransportCatalogue::ComputeActualDistance(const detail::Bus* bus) const {
		uint64_t actual_distance{};
		for (size_t i{ 1 }; i < bus->stops.size(); ++i) {
			if (stops_distance_.find({ bus->stops[i - 1], bus->stops[i] }) != stops_distance_.end()) {
				actual_distance += stops_distance_.at({ bus->stops[i - 1], bus->stops[i] });
			}
			else if (stops_distance_.find({ bus->stops[i], bus->stops[i - 1] }) != stops_distance_.end()) {
				actual_distance += stops_distance_.at({ bus->stops[i], bus->stops[i - 1] });
			}

			if (!bus->is_circle) {
				if (stops_distance_.find({ bus->stops[i], bus->stops[i - 1] }) != stops_distance_.end()) {
					actual_distance += stops_distance_.at({ bus->stops[i], bus->stops[i - 1] });
				}
				else if (stops_distance_.find({ bus->stops[i - 1], bus->stops[i] }) != stops_distance_.end()) {
					actual_distance += stops_distance_.at({ bus->stops[i - 1], bus->stops[i] });
				}
			}
		}
		return actual_distance;
	}

	void TransportCatalogue::FillStopnameToBuses(detail::Bus* bus) {
		for (const detail::Stop* stop : bus->stops) {
			stopname_to_buses_.at(stop->name).insert(std::move(bus));
		}
	}
}