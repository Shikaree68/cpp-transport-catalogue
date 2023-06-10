#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

void PrintJSON(const json::Node& node, std::ostream& out) {
    Print(json::Document{ node }, out);
}

JSONFacade::JSONFacade(TC::TransportCatalogue& db, std::istream& input) 
    : db_(db), node_(json::Load(input).GetRoot()), input_(input) {
    if (node_.IsDict()) {
        FillDB();
    }
}

void JSONFacade::FillDB() {
    std::vector<std::pair<std::pair<std::string, std::string>, int>> parced_distances; //контейнер для последующего добавления дистаний
    std::vector<const json::Node*> bus_node_ptr; //контейнер для последующей обработки маршрутов
    const json::Array& json_file{ node_.AsDict().at("base_requests").AsArray() }; //Нужно создать хранилище для всех дальнейших обработок
    for (const json::Node& data : json_file) {
        const json::Dict& stop_info{ data.AsDict() };
        if (stop_info.at("type"s).AsString() == "Stop"sv) {
            const std::string& stop_name{ stop_info.at("name"s).AsString() };
            //возможно нужна проверка на отсутстиве расстояний!
            for (auto& [to_stop, distance] : stop_info.at("road_distances"s).AsDict()) {
                parced_distances.emplace_back(std::make_pair(std::make_pair(stop_name, to_stop), distance.AsInt())); //добавляем пару остановок и дистанцию между ними
            }
            db_.AddStop({ stop_name, { stop_info.at("latitude"s).AsDouble(), stop_info.at("longitude"s).AsDouble() } });
        }
        else {
            //добавление указателей на автобсы в вектор для последующей обработки
            bus_node_ptr.push_back(&data);
        }        
    }
    //после первой итерации добавлены все остановки и заполнены контейнеры bus_db_ptr, parced_distances
    for (const auto& [stops, dist] : parced_distances) {
        db_.SetDistance(std::make_pair(db_.FindStop(stops.first), db_.FindStop(stops.second)), dist);
    }
    AddBuses(bus_node_ptr);
}

void JSONFacade::AddBuses(std::vector<const json::Node*>& buses) {
    for (const json::Node* bus_node_dict : buses) {
        const json::Dict& bus_info{ bus_node_dict->AsDict() };

        std::vector<const TC::detail::Stop*> bus_stops;
        for (const json::Node& bus_stop : bus_info.at("stops").AsArray()) {
            bus_stops.push_back(db_.FindStop(bus_stop.AsString()));
        }
        db_.AddBus({ bus_info.at("name"s).AsString(), bus_stops, bus_info.at("is_roundtrip"s).AsBool() });
    }
}

svg::Color JSONFacade::SetColor(const json::Node& color) {

    if (color.IsArray()) {
        //rgba
        if (color.AsArray().size() == 4) {
            return svg::Rgba
            {static_cast<uint32_t>(color.AsArray()[0].AsInt())
            ,static_cast<uint32_t>(color.AsArray()[1].AsInt())
            ,static_cast<uint32_t>(color.AsArray()[2].AsInt())
            ,color.AsArray()[3].AsDouble() };
        }
        //rgb
        else if (color.AsArray().size() == 3) {
            return svg::Rgb
            {static_cast<uint32_t>(color.AsArray()[0].AsInt())
            ,static_cast<uint32_t>(color.AsArray()[1].AsInt())
            ,static_cast<uint32_t>(color.AsArray()[2].AsInt())};
        }
    }
    //string
    else if (color.IsString()) {
        return color.AsString();
    }
    //monotype
    return nullptr;
    
}

std::vector<svg::Color> JSONFacade::SetVectorColor(const json::Array& color_array) {
    std::vector<svg::Color> v_color;
    v_color.reserve(color_array.size());
    for (const json::Node& color : color_array) {
        v_color.push_back(SetColor(color));
    }
    return v_color;
}

renderer::RenderSettings JSONFacade::GetRenderSettings() {
    const json::Dict& data{ node_.AsDict().at("render_settings"s).AsDict() };

    return renderer::RenderSettings{
         std::move(data.at("width"s).AsDouble())
        ,std::move(data.at("height"s).AsDouble())
        ,std::move(data.at("padding"s).AsDouble())
        ,std::move(data.at("line_width"s).AsDouble())
        ,std::move(data.at("stop_radius"s).AsDouble())
        ,std::move(data.at("bus_label_font_size"s).AsInt())
        ,{ std::move(data.at("bus_label_offset"s).AsArray()[0].AsDouble())
        ,std::move(data.at("bus_label_offset"s).AsArray()[1].AsDouble()) }
        ,std::move(data.at("stop_label_font_size"s).AsInt())
        ,{ std::move(data.at("stop_label_offset"s).AsArray()[0].AsDouble())
        ,std::move(data.at("stop_label_offset"s).AsArray()[1].AsDouble()) }
        ,{ SetColor(data.at("underlayer_color"s)) }
        ,std::move(data.at("underlayer_width"s).AsDouble())
        ,{ SetVectorColor(data.at("color_palette"s).AsArray()) }
    };
    
}

void JSONFacade::HandleRequests(std::ostream& output) {
    json::Array final_array{};

    for (const json::Node& data : node_.AsDict().at("stat_requests"s).AsArray()) {
        const json::Dict& request_info{ data.AsDict() };
        if (request_info.at("type"s).AsString() == "Stop"s) {
            const std::string& name{ request_info.at("name"s).AsString() };
            if (db_.FindStop(name)) { // нужно вызывать через request_handler?
                auto* buses{ db_.GetBusesByStop(name) };                
                std::vector<std::string> bus_names;
                if (buses) {
                    bus_names.reserve(buses->size());
                    for (const TC::detail::Bus* bus : *buses) {
                        bus_names.push_back(bus->name);
                    }
                }
                json::Node stop_node {
                    json::Builder{}
                        .StartDict()
                            .Key("buses"s).Value(json::Array{ bus_names.begin(), bus_names.end() })
                            .Key("request_id"s).Value(request_info.at("id"s))
                        .EndDict()
                    .Build()
                };


                //json::Node stop_node{
                //    json::Dict {{"buses"s, json::Array{bus_names.begin(), bus_names.end()} }
                //               ,{"request_id"s, request_info.at("id"s) }
                //    }
                //};
                final_array.push_back(stop_node);                
            }
            else {
                json::Node bus_node {
                    json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(request_info.at("id"s))
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                    .Build()
                };
                //json::Node bus_node{
                //  json::Dict {{"request_id"s, request_info.at("id"s) }
                //             ,{"error_message"s, "not found"s }
                //  }
                //};
                final_array.push_back(std::move(bus_node));
            }
        }
        else if(request_info.at("type"s).AsString() == "Bus"s){
            const TC::detail::Bus* bus{ db_.FindBus(request_info.at("name"s).AsString()) };
            if (bus) {
                json::Node bus_node {
                    json::Builder{}
                        .StartDict()
                            .Key("curvature"s).Value(bus->statistic.curveture)
                            .Key("request_id"s).Value(request_info.at("id"s))
                            .Key("route_length"s).Value(static_cast<int>(bus->statistic.actual_distance))
                            .Key("stop_count"s).Value(bus->statistic.total_stops)
                            .Key("unique_stop_count"s).Value(bus->statistic.unique_stops)
                        .EndDict()
                    .Build()
                };
                //json::Node bus_node {                  
                //    json::Dict {{"curvature"s        , bus->statistic.curveture }
                //               ,{"request_id"s       , request_info.at("id"s) }
                //               ,{"route_length"s     , static_cast<int>(bus->statistic.actual_distance) }
                //               ,{"stop_count"s       , bus->statistic.total_stops }
                //               ,{"unique_stop_count"s, bus->statistic.unique_stops }
                //    }
                //};
                final_array.push_back(std::move(bus_node));

            }
            else {
                json::Node bus_node {
                    json::Builder{}
                        .StartDict()
                            .Key("request_id"s).Value(request_info.at("id"s))
                            .Key("error_message"s).Value("not found"s)
                        .EndDict()
                    .Build()
                };
                //json::Node bus_node{
                //    json::Dict {{"request_id"s   , request_info.at("id"s) }
                //               ,{"error_message"s, "not found"s }}                    
                //};
                final_array.push_back(std::move(bus_node));
            }
        }
        else {
            std::stringstream map_rended;
            renderer::MapRenderer(map_rended, GetRenderSettings(), db_.GetAllBusesSorted());
            json::Node map_node {
                json::Builder{}
                    .StartDict()
                        .Key("map"s).Value(map_rended.str())
                        .Key("request_id"s).Value(request_info.at("id"s))
                    .EndDict()
                .Build()
            };
            /*json::Node map_node{
                json::Dict{{"map"s, map_rended.str() }
                          ,{"request_id"s, request_info.at("id"s)}}
            }*/;
            final_array.push_back(std::move(map_node));
        }
    }
    json::PrintNode(json::Node(std::move(final_array)), output);
}
