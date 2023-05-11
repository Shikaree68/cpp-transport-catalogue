#include "geo.h"
#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include <fstream>


int main() {
	TC::TransportCatalogue tc;
	TC::ReadData(tc);
}