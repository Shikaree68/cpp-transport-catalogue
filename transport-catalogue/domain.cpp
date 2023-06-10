#include "domain.h"

size_t TC::detail::BusPtrHash::operator()(Bus* bus) const {
	return str_hasher(bus->name) + hasher(bus) * 37 * 37;
}

size_t TC::detail::StopsPtrHash::operator()(std::pair<const Stop*, const Stop*> stops) const {
	return hasher(stops.first) + hasher(stops.second) * 37 * 37;
}

bool TC::detail::BusComparator::operator()(const Bus* lbus, const Bus* rbus) const {
	return lbus->name < rbus->name;
}

bool TC::detail::StopComparator::operator()(const Stop* lstop, const Stop* rstop) const {
	return lstop->name < rstop->name;
}
