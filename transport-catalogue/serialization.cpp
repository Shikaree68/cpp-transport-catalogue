#include "serialization.h"

namespace proto {

	Protobuf::Protobuf(TC::TransportCatalogue& db, renderer::RenderSettings rs, router::RoutingSettings route_settings) {
		for (auto& stop : db.GetAllStops()) {
			TC_proto::Stop p_stop;
			p_stop.set_stop_name(stop.name);

			TC_proto::Coordinates p_coordinates;
			p_coordinates.set_lat(stop.coordinates.lat);
			p_coordinates.set_lng(stop.coordinates.lng);

			*p_stop.mutable_coordinates() = p_coordinates;
			p_stop.set_id(stop.id);
			*db_.add_stops() = p_stop;
		}

		for (auto& bus : db.GetAllBuses()) {
			TC_proto::Bus p_bus;
			p_bus.set_bus_name(bus.name);
			p_bus.set_is_roundtrip(bus.is_circle);
			for (auto& stop : bus.stops) {
				p_bus.add_route(stop->id);
			}
			*db_.add_buses() = p_bus;
		}

		for (auto& [stops, distance] : db.GetAllDistances()) {
			TC_proto::Distance p_distance;
			p_distance.set_stop_from(stops.first->id);
			p_distance.set_stop_to(stops.second->id);
			p_distance.set_distance(distance);
			*db_.add_stops_distance() = p_distance;
		}

		SerializeRenderSettings(rs);
		rs_.set_bus_wait_time(route_settings.bus_wait_time);
		rs_.set_bus_velocity(route_settings.bus_velocity);
	}

	void Protobuf::Serialization(const std::filesystem::path& path) {
		std::ofstream out_file(path, std::ios::binary);
		db_.SerializeToOstream(&out_file);
		mr_.SerializeToOstream(&out_file);
		rs_.SerializeToOstream(&out_file);
	}

	void Protobuf::Deserialization(const std::filesystem::path& path) {
		std::ifstream in_file(path, std::ios::binary);
		db_.ParseFromIstream(&in_file);
		mr_.ParseFromIstream(&in_file);
		rs_.ParseFromIstream(&in_file);
	}
	void Protobuf::ParseCatalogue(TC::TransportCatalogue& tc) {
		ParseStops(tc);
		ParseDistances(tc);
		ParseBuses(tc);
		ParseRenderSettings();
	}

	void Protobuf::ParseStops(TC::TransportCatalogue& tc)
	{
		for (size_t i = 0; i < db_.stops_size(); ++i) {
			TC::detail::Stop stop;
			stop.id = db_.mutable_stops(i)->id();
			stop.name = db_.mutable_stops(i)->stop_name();
			stop.coordinates.lat = db_.mutable_stops(i)->coordinates().lat();
			stop.coordinates.lng = db_.mutable_stops(i)->coordinates().lng();
			tc.AddStop(std::move(stop));
			const TC::detail::Stop& last_stop{ tc.GetAllStops().back()};
			id_to_stop_.emplace(last_stop.id, &last_stop);
		}
	}

	void Protobuf::ParseDistances(TC::TransportCatalogue& tc) {
		for (auto& distance : db_.stops_distance()) {
			tc.SetDistance(std::make_pair(
				id_to_stop_[distance.stop_from()]
				, id_to_stop_[distance.stop_to()])
				, distance.distance());
		}
	}

	void Protobuf::ParseBuses(TC::TransportCatalogue& tc) {
		for (size_t i = 0; i < db_.buses_size(); ++i) {
			auto p_bus{ db_.mutable_buses(i) };
			TC::detail::Bus bus;
			bus.is_circle = p_bus->is_roundtrip();
			bus.name = p_bus->bus_name();

			for (size_t j = 0; j < p_bus->route_size(); ++j) {
				bus.stops.push_back(id_to_stop_.at(p_bus->route()[j]));
			}
			tc.AddBus(std::move(bus));
		}
	}
	renderer::RenderSettings Protobuf::ParseRenderSettings()
	{
		renderer::RenderSettings rs;
		rs.width = mr_.width();
		rs.height = mr_.height();
		rs.padding = mr_.padding();
		rs.line_width = mr_.line_width();
		rs.stop_radius = mr_.stop_radius();
		rs.bus_label_font_size = mr_.bus_label_font_size();
		rs.bus_label_offset = { mr_.bus_label_offset().x(), mr_.bus_label_offset().y() };
		rs.stop_label_font_size = mr_.stop_label_font_size();
		rs.stop_label_offset = { mr_.stop_label_offset().x(), mr_.stop_label_offset().y() };
		{
			if (mr_.underlayer_color().data_case() == 1) {
				rs.underlayer_color = std::monostate();
			}
			else if (mr_.underlayer_color().data_case() == 2) {
				rs.underlayer_color = mr_.underlayer_color().name();
			}
			else if (mr_.underlayer_color().data_case() == 3) {
				auto color{ mr_.underlayer_color().rgba() };
				if (mr_.underlayer_color().rgba().is_rgba()) {
					rs.underlayer_color = svg::Rgba{ color.red(), color.green(), color.blue(), color.opacity() };
				}
				else {
					rs.underlayer_color = svg::Rgb{ color.red(), color.green(), color.blue() };
				}
			}

		}
		rs.underlayer_width = mr_.underlayer_width();
		rs.color_palette.reserve(mr_.color_palette_size());
		for (auto& color : mr_.color_palette()) {
			if (color.data_case() == 1) {
				rs.color_palette.push_back( std::monostate());
			}
			else if (color.data_case() == 2) {
				rs.color_palette.push_back(color.name());
			}
			else if (color.data_case() == 3) {
				auto rs_color{ color.rgba() };
				if (color.rgba().is_rgba()) {
					rs.color_palette.push_back(svg::Rgba{ rs_color.red(), rs_color.green(), rs_color.blue(), rs_color.opacity() });
				}
				else {
					rs.color_palette.push_back(svg::Rgb{ rs_color.red(), rs_color.green(), rs_color.blue() });
				}
			}
		}
		return rs;

	}
	router::RoutingSettings Protobuf::ParseRoutingSettings() {
		return {rs_.bus_wait_time(), rs_.bus_velocity() };
	}
	void Protobuf::SerializeRenderSettings(renderer::RenderSettings rs) {

		mr_.set_width(rs.width);
		mr_.set_height(rs.height);
		mr_.set_padding(rs.padding);
		mr_.set_line_width(rs.line_width);
		mr_.set_stop_radius(rs.stop_radius);
		mr_.set_bus_label_font_size(rs.bus_label_font_size);
		mr_.mutable_bus_label_offset()->set_x(rs.bus_label_offset.x);
		mr_.mutable_bus_label_offset()->set_y(rs.bus_label_offset.y);
		mr_.set_stop_label_font_size(rs.stop_label_font_size);
		mr_.mutable_stop_label_offset()->set_x(rs.stop_label_offset.x);
		mr_.mutable_stop_label_offset()->set_y(rs.stop_label_offset.y);

		if (std::holds_alternative<std::string>(rs.underlayer_color)) {
			mr_.mutable_underlayer_color()->set_name(std::get<std::string>(rs.underlayer_color));
		}
		else if (std::holds_alternative<svg::Rgb>(rs.underlayer_color) ) {
			proto_render::Rgba p_color;
			svg::Rgb rs_color{std::get<svg::Rgb>(rs.underlayer_color)};
			p_color.set_red(rs_color.red);
			p_color.set_green(rs_color.green);
			p_color.set_blue(rs_color.blue);
			p_color.set_is_rgba(false);
			
			*mr_.mutable_underlayer_color()->mutable_rgba() = p_color;
		}
		else if (std::holds_alternative<svg::Rgba>(rs.underlayer_color)) {
			proto_render::Rgba p_color;
			svg::Rgba rs_color{std::get<svg::Rgba>(rs.underlayer_color)};
			p_color.set_red(rs_color.red);
			p_color.set_green(rs_color.green);
			p_color.set_blue(rs_color.blue);
			p_color.set_opacity(rs_color.opacity);
			p_color.set_is_rgba(true);

			*mr_.mutable_underlayer_color()->mutable_rgba() = p_color;
		}
		else {
			mr_.mutable_underlayer_color()->set_is_none(true);
		}

		mr_.set_underlayer_width(rs.underlayer_width);

		for (auto& rs_color : rs.color_palette) {
			auto add_collor{ mr_.add_color_palette() };
			if (std::holds_alternative<std::string>(rs_color)) {
				add_collor->set_name(std::get<std::string>(rs_color));
			}
			else if (std::holds_alternative<svg::Rgb>(rs_color) ) {
				proto_render::Rgba p_color;
				svg::Rgb color{std::get<svg::Rgb>(rs_color)};
				p_color.set_red(color.red);
				p_color.set_green(color.green);
				p_color.set_blue(color.blue);
				p_color.set_is_rgba(false);				
				*add_collor->mutable_rgba() = p_color;
			}
			else if (std::holds_alternative<svg::Rgba>(rs_color)) {
				proto_render::Rgba p_color;
				svg::Rgba color{std::get<svg::Rgba>(rs_color)};
				p_color.set_red(color.red);
				p_color.set_green(color.green);
				p_color.set_blue(color.blue);
				p_color.set_opacity(color.opacity);
				p_color.set_is_rgba(true);
				*add_collor->mutable_rgba() = p_color;
			}
			else {
				add_collor->set_is_none(true);
			}
		}
	}
}
