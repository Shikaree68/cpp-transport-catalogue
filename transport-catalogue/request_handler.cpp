#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

RequestHandler::RequestHandler(const TC::TransportCatalogue& db, const renderer::MapRenderer& renderer) 
	: db_(db)
	, renderer_(renderer) {
}

std::optional<const TC::detail::Bus*> RequestHandler::GetBusStat(const std::string_view& bus_name) const  {
	const TC::detail::Bus* bus_ptr{ db_.FindBus(bus_name) };
	if (bus_ptr) {
		return bus_ptr;
	}
	return {};
}

const std::set<TC::detail::Bus*, TC::detail::BusComparator>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const{
	return db_.GetBusesByStop(stop_name);
}
