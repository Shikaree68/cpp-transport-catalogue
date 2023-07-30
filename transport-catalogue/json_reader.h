#pragma once

#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>
#include <iostream>

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"

void PrintJSON(const json::Node& node, std::ostream& out);


class JSONFacade {
public:
	JSONFacade() = delete;
	explicit JSONFacade(TC::TransportCatalogue& db, std::istream& input = std::cin);
	explicit JSONFacade(std::istream& input = std::cin);
	//Отдаем настройки рендера
	renderer::RenderSettings GetRenderSettings();
	router::RoutingSettings GetRoutingSettings() const;
	void SetRenderSettings(renderer::RenderSettings rs);
	void SetRoutingSettings(router::RoutingSettings rs);
	//Выводим все запросы
	void HandleRequests(std::ostream& output = std::cout);
	std::string GetSerializationPath();
	void FillDB();


private:
	TC::TransportCatalogue& db_;
	json::Node node_;
	std::istream& input_;
	renderer::RenderSettings render_settings_;
	router::RoutingSettings routing_settings_;

	void FillStops(std::vector<const json::Node*>& bus_node_ptr,
		std::vector<std::pair<std::pair<std::string, std::string>, int>>& parced_distances);
	void FillBuses(const std::vector<const json::Node*>& buses,
		const std::vector<std::pair<std::pair<std::string, std::string>, int>>& distances);
	svg::Color SetColor(const json::Node& color);
	std::vector<svg::Color> SetVectorColor(const json::Array& color_array);
	void HandleRequestStop(const json::Dict& request_info, json::Array& final_array);
	void HandleRequestBus(const json::Dict& request_info, json::Array& final_array);
	void HandleRequestRoute(router::TransportRouter& tr, const json::Dict& request_info, json::Array& final_array);
	void HandleRequestMap(const json::Dict& request_info, json::Array& final_array);
	void HandleNotFound(const json::Dict& request_info, json::Array& final_array);
};