#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include <fstream>
#include <iostream>


int main() {
	TC::TransportCatalogue tc;
	TC::ReadData(std::cin, tc);
	TC::HandleQuery(tc);
}