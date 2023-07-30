#include "json_reader.h"

using namespace std::string_view_literals;
using namespace std::string_literals;

void PrintJSON(const json::Node& node, std::ostream& out) {
    Print(json::Document{ node }, out);
}

JSONFacade::JSONFacade(TC::TransportCatalogue& db, std::istream& input) 
    : db_(db), node_(json::Load(input).GetRoot()), input_(input) {}

void JSONFacade::FillDB() {
    //контейнер для последующего добавления дистаний
    std::vector<std::pair<std::pair<std::string, std::string>, int>> distances; 
    std::vector<const json::Node*> buses; //контейнер для последующей обработки маршрутов    
    FillStops(buses, distances);
    //после первой итерации добавлены все остановки и заполнены контейнеры buses, distances
    FillBuses(buses, distances);
}

//заполняем БД остановками и готовим автобусы и расстояния
void JSONFacade::FillStops(std::vector<const json::Node*>& bus_node_ptr,
    std::vector<std::pair<std::pair<std::string, std::string>, int>>& distances) {
    //Нужно создать хранилище для всех дальнейших обработок
    const json::Array& json_file{ node_.AsDict().at("base_requests").AsArray() }; 
    size_t stop_id{};
    for (const json::Node& data : json_file) {
        const json::Dict& stop_info{ data.AsDict() };
        if (stop_info.at("type"s).AsString() == "Stop"sv) {
            const std::string& stop_name{ stop_info.at("name"s).AsString() };
            //возможно нужна проверка на отсутстиве расстояний!
            for (auto& [to_stop, distance] : stop_info.at("road_distances"s).AsDict()) {
                //добавляем пару остановок и дистанцию между ними
                distances.emplace_back(std::make_pair(std::make_pair(stop_name, to_stop), distance.AsInt())); 
            }
            db_.AddStop({ stop_name, { stop_info.at("latitude"s).AsDouble(), stop_info.at("longitude"s).AsDouble() }, stop_id });
            stop_id += 2;
        }
        else {
            //добавление указателей на автобсы в вектор для последующей обработки
            bus_node_ptr.push_back(&data);
        }
    }
}

void JSONFacade::FillBuses(const std::vector<const json::Node*>& buses, 
    const std::vector<std::pair<std::pair<std::string, std::string>, int>>& distances) {
    for (const auto& [stops, dist] : distances) {
        db_.SetDistance(std::make_pair(db_.FindStop(stops.first), db_.FindStop(stops.second)), dist);
    }
    for (const json::Node* bus_node_dict : buses) {
        const json::Dict& bus_info{ bus_node_dict->AsDict() };
        std::vector<const TC::detail::Stop*> bus_stops;
        for (const json::Node& bus_stop : bus_info.at("stops").AsArray()) {
            bus_stops.push_back(db_.FindStop(bus_stop.AsString()));
        }
        db_.AddBus({ bus_info.at("name"s).AsString(), bus_stops, bus_info.at("is_roundtrip"s).AsBool()});
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

void JSONFacade::HandleRequests(std::ostream& output) {
    json::Array final_array{};
    router::TransportRouter tr(db_, routing_settings_);
    for (const json::Node& data : node_.AsDict().at("stat_requests"s).AsArray()) {
        const json::Dict& request_info{ data.AsDict() };
        if (request_info.at("type"s).AsString() == "Stop"s) {
            HandleRequestStop(request_info, final_array);  
        }
        else if(request_info.at("type"s).AsString() == "Bus"s){
            HandleRequestBus(request_info, final_array);            
        }
        else if (request_info.at("type"s).AsString() == "Map"s) {
            HandleRequestMap(request_info, final_array);
        }
        else if (request_info.at("type"s).AsString() == "Route"s) {
           HandleRequestRoute(tr, request_info, final_array);
        }
    }
    json::PrintNode(json::Node(std::move(final_array)), output);
}

std::string JSONFacade::GetSerializationPath() {
    return node_.AsDict().at("serialization_settings").AsDict().at("file"s).AsString();
}
void JSONFacade::HandleRequestStop(const json::Dict& request_info, json::Array& final_array) {
    const std::string& name{ request_info.at("name"s).AsString() };
    if (db_.FindStop(name)) {
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

        final_array.push_back(stop_node);
    }
    else {
        HandleNotFound(request_info, final_array);
    }
}

void JSONFacade::HandleRequestBus(const json::Dict& request_info, json::Array& final_array) {
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
        final_array.push_back(std::move(bus_node));
    }
    else {
        HandleNotFound(request_info, final_array);
    }
}

void JSONFacade::HandleRequestRoute(router::TransportRouter& tr, const json::Dict& request_info, json::Array& final_array){
    const TC::detail::Stop* from{db_.FindStop(request_info.at("from"s).AsString())};
    const TC::detail::Stop* to{db_.FindStop(request_info.at("to"s).AsString())};
    if (from && to) {
        auto route{ std::move(tr.GetEdgesInfo(from->id, to->id)) };
        if (!route.has_value()) {
            HandleNotFound(request_info, final_array);
        }
        else {
            double total_weight{};
            json::Array items;
            items.reserve(route.value().size());
            for (const router::EdgeInfo* edge : route.value()) {
                total_weight += edge->weight;
                if (edge->span_count == 0) {
                    items.push_back(json::Node{
                        json::Builder{}
                        .StartDict()
                            .Key("type"s).Value("Wait"s)
                            .Key("stop_name"s).Value(std::string(edge->name))
                            .Key("time"s).Value(edge->weight)
                        .EndDict()
                        .Build()
                    });
                }
                else {
                    items.push_back(json::Node{
                        json::Builder{}
                        .StartDict()
                            .Key("type"s).Value("Bus"s)
                            .Key("bus"s).Value(std::string(edge->name))
                            .Key("span_count"s).Value(edge->span_count)
                            .Key("time"s).Value(edge->weight)
                        .EndDict()
                        .Build()
                    });
                }
            }
            json::Node route_node {
                json::Builder{}
                .StartDict()
                    .Key("request_id"s).Value(request_info.at("id"s))
                    .Key("total_time"s).Value(std::move(total_weight))
                    .Key("items"s).Value(std::move(items))
                .EndDict()
                .Build()

            };
            final_array.push_back(std::move(route_node));
        }
    }
    else {
        HandleNotFound(request_info, final_array);
    }
}

void JSONFacade::HandleRequestMap(const json::Dict& request_info, json::Array& final_array) {
    std::stringstream map_rended;
    renderer::MapRenderer(map_rended, std::move(render_settings_), db_.GetAllBusesSorted());
    json::Node map_node {
        json::Builder{}
        .StartDict()
            .Key("map"s).Value(map_rended.str())
            .Key("request_id"s).Value(request_info.at("id"s))
        .EndDict()
        .Build()
    };
    final_array.push_back(std::move(map_node));
}

void JSONFacade::HandleNotFound(const json::Dict& request_info, json::Array& final_array) {
    json::Node route_node {
        json::Builder{}
        .StartDict()
            .Key("request_id"s).Value(request_info.at("id"s))
            .Key("error_message"s).Value("not found"s)
        .EndDict()
        .Build()
    };
    final_array.push_back(std::move(route_node));
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

void JSONFacade::SetRenderSettings(renderer::RenderSettings rs) {
    render_settings_ = std::move(rs);
}

void JSONFacade::SetRoutingSettings(router::RoutingSettings rs)
{
    routing_settings_ = std::move(rs);
}

router::RoutingSettings JSONFacade::GetRoutingSettings() const {
    const json::Dict& data{ node_.AsDict().at("routing_settings"s).AsDict() };
    return { data.at("bus_wait_time"s).AsDouble(), data.at("bus_velocity"s).AsDouble() };
}
