#pragma once
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"
#include "geo.h"
#include <deque>
#include <unordered_map>

namespace router {
	struct EdgeInfo {
		std::string_view name{};
		int span_count{};
		double weight{};	

		EdgeInfo() = delete;

		EdgeInfo(const std::string_view& name, int span_count, double weight)
			: name(name), span_count(span_count), weight(weight) {}
		EdgeInfo(const EdgeInfo& other) {
			*this = other;
		}
		EdgeInfo& operator=(const router::EdgeInfo& other) {
			name = other.name;
			span_count = other.span_count;
			weight = other.weight;
			return *this;
		}
	};

	struct RoutingSettings {
		double bus_wait_time{};
		double bus_velocity{};
	};

	class TransportRouter {
	public:
		TransportRouter(TC::TransportCatalogue& tc, RoutingSettings routing_settings);
		TransportRouter() = delete;

		const graph::DirectedWeightedGraph<double>& GetGraph() {
			return dwg_;
		}
		const std::vector<EdgeInfo>& GetEdgesInfo() const {
			return route_;
		}

	private:
		TC::TransportCatalogue& tc_;
		RoutingSettings routing_settings_;
		graph::DirectedWeightedGraph<double> dwg_;
		std::vector<EdgeInfo> route_;

		using stops_time = std::unordered_map<std::pair<const TC::detail::Stop*, const TC::detail::Stop*>,
			double, TC::detail::StopsPtrHash>;
		const double COEF{ 16.6667 };
		void FillGraph();		
		void AddBusesToGraph();
		template <typename ItV>
		void AddBus(ItV begin, ItV end, std::string_view bus_name, stops_time& stops_time_var);

	};

	template <typename ItV>
	void TransportRouter::AddBus(ItV begin, ItV end, std::string_view bus_name, stops_time& stops_time_var) {
		{
			for (ItV it{ begin + 1 }; it != end; ++it) {
				stops_time_var[{*std::prev(it), *it }] =
					tc_.FindTwoStopsDistance({
						*std::prev(it), *it })
						/ routing_settings_.bus_velocity;
			}
			for (ItV it{ begin }; std::next(it) != end; ++it) {
				for (auto jt{ std::next(it) }; jt != end; ++jt) {
					double total_time{};
					for (auto zt{ it }; zt != jt; ++zt) {
						total_time += stops_time_var.at({ *zt, *(std::next(zt)) });
					}
					dwg_.AddEdge({
						(*it)->id + 1,
						(*jt)->id ,
						total_time });
					route_.emplace_back(bus_name, std::distance(it, jt), total_time);
				}
			}
		}
	}
}

