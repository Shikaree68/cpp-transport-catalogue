#include "input_reader.h"
#include <iostream>
#include <algorithm>

using namespace std::string_literals;
using namespace std::string_view_literals;

namespace TC {
	void ReadData(std::istream& input, TransportCatalogue& tc) {
		int query_inputs;
		input >> query_inputs;
		std::vector< std::string> bus_queries;
		std::vector< std::pair<std::string, std::string>> stop_distances;
		input.ignore();
		for (int i{}; i < query_inputs; ++i) {
			std::string query_type;
			getline(input, query_type, ' ');
			if (query_type == "Stop") {
				ParceStop(input, tc, stop_distances);
			}
			else {
				std::string bus;
				getline(input, bus);
				bus_queries.push_back(std::move(bus));
			}
		}
		std::vector<std::string_view> sv_bus_queries(bus_queries.begin(), bus_queries.end());
		std::vector< std::pair<std::string_view, std::string_view>> sv_stop_distances(stop_distances.begin(), stop_distances.end());
		ParceStopDistances(std::move(sv_stop_distances), tc);
		ParceBuses(std::move(sv_bus_queries), tc);	
	}

	void ParceStop(std::istream& input, TransportCatalogue& tc, std::vector< std::pair<std::string, std::string>>& stop_distances) {
		std::string stop_name;
		getline(input, stop_name, ':');
		std::string lat, lng, distances;
		getline(input, lat, ',');
		getline(input, distances);
		size_t comma = distances.find(',');
		if (comma != distances.npos) {
			lng = distances.substr(0, comma);
			distances = distances.substr(comma + 1);
			tc.AddStop({ stop_name, {std::move(stod(lat)), std::move(stod(lng)) } });
			stop_distances.push_back(std::make_pair(std::move(stop_name), distances));
		}
		else {
			tc.AddStop({ stop_name, {std::move(stod(lat)), std::move(stod(distances)) } });
		}
	}

	void ParceStopDistances(std::vector< std::pair<std::string_view, std::string_view>>&& distances, TransportCatalogue& tc) {
		for (auto& [stop, distance] : distances) {
			while (distance.size()) {
				std::string dist_value{ distance.substr(1, distance.find('m') - 1) };
				size_t fpos{ distance.find('o') + 2 };
				size_t spos{ distance.find(',') };
				std::string_view to_stop{ distance.substr(fpos, spos - fpos) };
				auto f_stop_ptr{ tc.FindStop(stop) };
				auto s_stop_ptr{ tc.FindStop(to_stop) };

				tc.SetDistance(std::make_pair(f_stop_ptr, s_stop_ptr), std::stoi(dist_value));
				if (spos == distance.npos) break;
				distance = distance.substr(spos + 1);
			}
		}
	}

	void ParceBuses(std::vector<std::string_view>&& buses, TransportCatalogue& tc) {
		for (std::string_view& bus : buses) {
			size_t pos{ bus.find(':') };
			std::string_view name{ bus.substr(0, pos) };
			std::string_view body{ bus.substr(pos + 1, bus.size()) };
			std::vector<detail::Stop*> stops;

			bool circle{ true };
			char symb;
			if (body.find('>') != body.npos) {
				symb = '>';
			}
			else {
				symb = '-';
				circle = false;
			}

			while (body.size()) {
				size_t symb_pos{ body.find(symb) };
				std::string_view bus_stop{ body.substr(1 ,symb_pos - 2) };
				stops.push_back(std::move(tc.FindStop(bus_stop)));
				if (symb_pos == body.npos) break;
				body = body.substr(symb_pos + 1);
			}

			tc.AddBus({ std::string(name), std::move(stops), circle });
		}
	}
}
