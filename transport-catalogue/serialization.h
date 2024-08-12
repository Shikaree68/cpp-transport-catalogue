#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <svg.pb.h>
#include <filesystem>
#include <fstream>


namespace proto {
	class Protobuf {
	public:
		Protobuf() = default;
		Protobuf(TC::TransportCatalogue& db, renderer::RenderSettings rs, router::RoutingSettings route_settings);

		void Serialization(const std::filesystem::path& path);
		void Deserialization(const std::filesystem::path& path);
		void ParseCatalogue(TC::TransportCatalogue& tc);
		renderer::RenderSettings ParseRenderSettings();
		router::RoutingSettings ParseRoutingSettings();
	private:
		TC_proto::TransportCatalogue db_;
		std::map<const uint64_t, const TC::detail::Stop*> id_to_stop_;

		void ParseStops(TC::TransportCatalogue& tc);
		void ParseDistances(TC::TransportCatalogue& tc);
		void ParseBuses(TC::TransportCatalogue& tc);


		void SerializeRenderSettings(renderer::RenderSettings rs);
	};
}