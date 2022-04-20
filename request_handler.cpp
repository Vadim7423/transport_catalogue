#include "request_handler.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
*/
using namespace transport_list;

RequestHandler::RequestHandler(const transport_list::TransportCatalogue& db, const renderer::MapRenderer& renderer)
    :db_(db), renderer_(renderer)
{}

domain::Bus* RequestHandler::GetBusStat(const std::string& bus_name) const {
    return db_.GetBusInfo(bus_name);
}

const std::optional<std::set<std::string>> RequestHandler::GetBusesByStop(const std::string& stop_name) const {
    return db_.GetStopInfo(stop_name);
}

void RequestHandler::RenderMap() {
    renderer_.RenderMap();
}
