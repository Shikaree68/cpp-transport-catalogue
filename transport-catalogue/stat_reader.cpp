#include "stat_reader.h"
#include <iostream>
#include <fstream>
using namespace std::string_literals;

namespace TC {
	void HandleQuery(TransportCatalogue& tc) {
		int count{};
		std::cin >> count;
		std::cin.ignore();
		std::ofstream fout("out.txt");
		for (int i{ 0 }; i < count; ++i) {
			std::string query, type;
			std::cin >> type;
			std::cin.ignore();
			getline(std::cin, query);
			if (type == "Bus"s) {
				tc.GetBusInfo(fout, query);
			}
			else if (type == "Stop"s) {
				tc.GetStopInfo(fout, query);
			}
		}
	}
}