#pragma once

#include <string>
#include <vector>
#include <iomanip>

#include "transport_catalogue.h"
#include "geo.h"

namespace TC {
	void ReadData(TransportCatalogue& tc);
	void ParceStop(TransportCatalogue& tc, std::vector< std::pair<std::string, std::string>>& stop_distances);
	void ParceStopDistances(std::vector< std::pair<std::string_view, std::string_view>>&& distances, TransportCatalogue& tc);
	void ParceBuses(std::vector<std::string_view>&& buses, TransportCatalogue& tc);
}