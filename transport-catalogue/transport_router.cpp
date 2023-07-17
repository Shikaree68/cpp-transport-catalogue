#include "transport_router.h"
#include <numeric>
namespace router {
	TransportRouter::TransportRouter(TC::TransportCatalogue& tc, RoutingSettings routing_settings)
		: tc_(tc), routing_settings_(routing_settings), dwg_(tc_.GetAllStops().size() * 2) {
		routing_settings_.bus_velocity = routing_settings_.bus_velocity * COEF;
		FillGraph();
		
	}
	void TransportRouter::FillGraph() {
		//добавляем ребра остановок и структуру
		const auto& stops{ tc_.GetAllStops() };
		for (const auto& stop : stops) {
			dwg_.AddEdge({
				stop.id ,
				stop.id + 1,
				routing_settings_.bus_wait_time });
			route_.emplace_back(stop.name, 0, routing_settings_.bus_wait_time);
		}
		AddBusesToGraph();		
	}

	void TransportRouter::AddBusesToGraph() {
		const auto& buses{ (tc_.GetAllBuses()) };
		for (const auto& bus : buses) {
			stops_time stops_time_var;
			AddBus(bus.stops.begin(), bus.stops.end(), bus.name, stops_time_var);

			if (!bus.is_circle) {
				AddBus(bus.stops.rbegin(), bus.stops.rend(), bus.name, stops_time_var);
			}
		}
	}

	

}