#include "json_reader.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <string_view>
#include <iostream>
#include <fstream>
#include "serialization.h"

using namespace std::string_view_literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {
        TC::TransportCatalogue tc;
        JSONFacade json(tc, std::cin);
        json.FillDB();
        proto::Protobuf proto(tc, json.GetRenderSettings(), json.GetRoutingSettings());
        proto.Serialization(json.GetSerializationPath());
    }
    else if (mode == "process_requests"sv) {
        TC::TransportCatalogue tc;
        JSONFacade json(tc, std::cin);
        proto::Protobuf proto;
        proto.Deserialization(json.GetSerializationPath());
        proto.ParseCatalogue(tc);
        json.SetRenderSettings(proto.ParseRenderSettings());
        json.SetRoutingSettings(proto.ParseRoutingSettings());
        json.HandleRequests(std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}