#pragma once

#include <cassert>
#include <chrono>
#include <sstream>
#include <string_view>

#include "json.h"
#include "request_handler.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_builder.h"


/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

void PrintJSON(const json::Node& node, std::ostream& out);

using namespace std::string_view_literals;
using namespace std::string_literals;



class JSONFacade {
public:
	JSONFacade() = delete;
	explicit JSONFacade(TC::TransportCatalogue& db, std::istream& input);
	//Отдаем настройки рендера
	renderer::RenderSettings GetRenderSettings();
	//Выводим все запросы
	void HandleRequests(std::ostream& output);

private:
	TC::TransportCatalogue& db_;
	json::Node node_;
	std::istream& input_;

	void FillDB();
	void AddBuses(std::vector<const json::Node*>& buses);
	svg::Color SetColor(const json::Node& color);
	std::vector<svg::Color> SetVectorColor(const json::Array& color_array);
};