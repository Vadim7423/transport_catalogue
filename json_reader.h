#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

class JsonReader {
public:
    void SetData(transport_list::TransportCatalogue& catalog,
                 renderer::MapRenderer& render,
                 std::istream& input);

    json::Array GetData(transport_list::TransportCatalogue& catalog,
                        renderer::MapRenderer& render);

private:
    json::Dict json_data_;
    json::Array input_requests_;
    json::Array output_requests_;
    json::Dict render_settings_;

    void SetStops(transport_list::TransportCatalogue& catalog);
    void SetBuses(transport_list::TransportCatalogue& catalog);
    void SetDistances(transport_list::TransportCatalogue& catalog);
    void SetSetRenderSettings(renderer::MapRenderer& render);
    void SetColorPalette(renderer::MapRenderer& render);
    void SetUnderLayerColor(renderer::MapRenderer& render);

    json::Dict GetBusInfo(transport_list::TransportCatalogue& catalog, const json::Dict& request);
    json::Dict GetStopInfo(transport_list::TransportCatalogue& catalog, const json::Dict& request);
    json::Dict GetMap(const renderer::MapRenderer& map, const json::Dict& request);
    double GetCurvature(const domain::Bus* bus, int real_distance);
};
