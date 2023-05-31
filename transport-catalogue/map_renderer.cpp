#include "map_renderer.h"
#include <map>

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
using namespace std::string_literals;
namespace renderer {
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    void MapRenderer(std::ostream& output, renderer::RenderSettings&& settings
        , std::set<const TC::detail::Bus*, TC::detail::BusComparator>&& routes) {
        const RenderSettings render_settings{ std::move(settings) };

        std::set<const TC::detail::Stop*,TC::detail::StopComparator> stops_for_render;
        std::vector<geo::Coordinates> coordinates;
        for (const TC::detail::Bus* bus : routes) {
            for (const TC::detail::Stop* stop : bus->stops) {
                coordinates.push_back(stop->coordinates);
                stops_for_render.insert(stop);
            }
        }       

        SortAndClean(coordinates);

        const SphereProjector sphere_projector_{ 
              coordinates.begin()
            , coordinates.end()
            , render_settings.width
            , render_settings.height
            , render_settings.padding
        };

        std::map<std::string, svg::Document> rended_routes;
        
        for (const TC::detail::Stop* stop : stops_for_render) {
            rended_routes["stop_points"s].Add(svg::Circle()
                                         .SetCenter(sphere_projector_(stop->coordinates))
                                         .SetRadius(render_settings.stop_radius)
                                         .SetFillColor("white"s));
            //подложка
            rended_routes["stop_names"s].Add(svg::Text()
                                        .SetPosition(sphere_projector_(stop->coordinates))
                                        .SetOffset(render_settings.stop_label_offset)
                                        .SetFontSize(render_settings.stop_label_font_size)
                                        .SetFontFamily("Verdana"s)
                                        .SetData(stop->name)
                                        .SetFillColor(render_settings.underlayer_color)
                                        .SetStrokeColor(render_settings.underlayer_color)
                                        .SetStrokeWidth(render_settings.underlayer_width)
                                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            //само название
            rended_routes["stop_names"s].Add(svg::Text()
                                        .SetPosition(sphere_projector_(stop->coordinates))
                                        .SetOffset(render_settings.stop_label_offset)
                                        .SetFontSize(render_settings.stop_label_font_size)
                                        .SetFontFamily("Verdana"s)
                                        .SetData(stop->name)
                                        .SetFillColor("black"s));
        }
        size_t color_index{};
        //Создаем маршруты
        for (const TC::detail::Bus* bus : routes) {
            if (color_index >= render_settings.color_palette.size()) {
                color_index = 0;
            }
            svg::Polyline svg_route;
            //добавляем название на начальной точке маршрута в отрисовку
            //подложка
            rended_routes["bus_names"s].Add(svg::Text()
                                       .SetPosition(sphere_projector_(bus->stops[0]->coordinates))
                                       .SetOffset(render_settings.bus_label_offset)
                                       .SetFontSize(render_settings.bus_label_font_size)
                                       .SetFontFamily("Verdana"s)
                                       .SetFontWeight("bold"s)
                                       .SetData(bus->name)
                                       .SetFillColor(render_settings.underlayer_color)
                                       .SetStrokeColor(render_settings.underlayer_color)
                                       .SetStrokeWidth(render_settings.underlayer_width)
                                       .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                       .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
             
            //текст
            rended_routes["bus_names"s].Add(svg::Text()
                                       .SetPosition(sphere_projector_(bus->stops[0]->coordinates))
                                       .SetOffset(render_settings.bus_label_offset)
                                       .SetFontSize(render_settings.bus_label_font_size)
                                       .SetFontFamily("Verdana"s)
                                       .SetFontWeight("bold"s)
                                       .SetFillColor(render_settings.color_palette.at(color_index))    
                                       .SetData(bus->name));
            //создание линий маршрутов
            for (const TC::detail::Stop* stop : bus->stops) {
                svg_route.AddPoint(sphere_projector_(stop->coordinates));
            }
            //если маршрут не кольцевой создаем обратный путь
            if (!bus->is_circle) {
                if (bus->stops[0] != bus->stops[bus->stops.size() - 1]) {
                    //подложка
                    rended_routes["bus_names"s].Add(svg::Text()
                                               .SetPosition(sphere_projector_(bus->stops[bus->stops.size() - 1]->coordinates))
                                               .SetOffset(render_settings.bus_label_offset)
                                               .SetFontSize(render_settings.bus_label_font_size)
                                               .SetFontFamily("Verdana"s)
                                               .SetFontWeight("bold"s)
                                               .SetData(bus->name)
                                               .SetFillColor(render_settings.underlayer_color)
                                               .SetStrokeColor(render_settings.underlayer_color)
                                               .SetStrokeWidth(render_settings.underlayer_width)
                                               .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                                               .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
                    //добавляем название маршрута на конечной точки маршрута
                    rended_routes["bus_names"s].Add(svg::Text()
                                               .SetPosition(sphere_projector_(bus->stops[bus->stops.size() - 1]->coordinates))
                                               .SetOffset(render_settings.bus_label_offset)
                                               .SetFontSize(render_settings.bus_label_font_size)
                                               .SetFontFamily("Verdana"s)
                                               .SetFontWeight("bold"s)
                                               .SetFillColor(render_settings.color_palette.at(color_index))
                                               .SetData(bus->name));
                }
                    //создаем обратный путь
                for (auto it = std::next(bus->stops.rbegin()); it != bus->stops.rend(); ++it) {
                    svg_route.AddPoint(sphere_projector_((*it)->coordinates));
                }
            }

            //отрисовка путей
            svg_route.SetStrokeColor(render_settings.color_palette.at(color_index))
                     .SetFillColor(svg::NoneColor).SetStrokeWidth(render_settings.line_width)
                     .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                     .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            ++color_index;

            rended_routes["routes"s].Add(std::move(svg_route));
        }
        PrintMap(output, rended_routes);
    }
    //сортируем и избавляемся от лишних координат для отрисовки без повторений
    void SortAndClean(std::vector<geo::Coordinates>& coordinates) {
        std::sort(coordinates.begin(), coordinates.end());
        auto last = std::unique(coordinates.begin(), coordinates.end());
        coordinates.erase(last, coordinates.end());
    }

    void PrintMap(std::ostream& output, std::map<std::string, svg::Document>& rended_routes) {
        if (!rended_routes.empty()) {
            rended_routes.at("routes"s)
                         .AddDocument(std::move(rended_routes.at("bus_names"s)))
                         .AddDocument(std::move(rended_routes.at("stop_points"s)))
                         .AddDocument(std::move(rended_routes.at("stop_names"s)))
                         .Render(output);
        }
        else {
            svg::Document empty_doc;
            empty_doc.Render(output);
        }
    }
}
