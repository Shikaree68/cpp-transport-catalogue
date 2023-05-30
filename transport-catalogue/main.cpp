#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <string_view>
#include <iostream>
#include <fstream>

using namespace std::string_view_literals;

int main() {
    TC::TransportCatalogue tc;
    JSONFacade json(tc, std::cin);
    json.HandleRequests(std::cout);
}