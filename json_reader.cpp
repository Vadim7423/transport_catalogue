#include <sstream>
#include <algorithm>
#include <variant>

#include "json_reader.h"

/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

using namespace std;
using namespace json;
using namespace transport_list;
using namespace renderer;

std::string Print(const Node& node) {
    std::ostringstream out;
    Print(Document{node}, out);
    return out.str();
}

void JsonReader::SetData(TransportCatalogue& catalog,
                         MapRenderer& render,
                         std::istream& input) {
    json_data_ = json::Load(input).GetRoot().AsMap();
    SetStops(catalog);
    SetBuses(catalog);
    SetDistances(catalog);
    SetSetRenderSettings(render);
}

json::Array JsonReader::GetData(TransportCatalogue& catalog,
                                renderer::MapRenderer& render) {
    json::Array result;

    if(json_data_.find("stat_requests"s) == json_data_.end()) {
        return result;
    }

    output_requests_ = json_data_.at("stat_requests"s).AsArray();

    for(const auto& req : output_requests_) {
        if(req.AsMap().at("type"s) == "Bus"s) {
            result.push_back(GetBusInfo(catalog, req.AsMap()));
        } else if(req.AsMap().at("type"s) == "Stop"s) {
            result.push_back(GetStopInfo(catalog, req.AsMap()));
        } else if(req.AsMap().at("type"s) == "Map"s) {
            result.push_back(GetMap(render, req.AsMap()));
        }
    }
cout << Print(result) << endl;
    return result;
}

void JsonReader::SetStops(TransportCatalogue& catalog) {
    if(json_data_.find("base_requests"s) == json_data_.end()) {
        return;
    }

    input_requests_ = json_data_.at("base_requests"s).AsArray();

    std::sort(input_requests_.begin(),
              input_requests_.end(),
              [](const auto& item1, const auto& item2){
                    return item1.AsMap().at("type") != item2.AsMap().at("type")
                             ? item1.AsMap().at("type").AsString() > item2.AsMap().at("type").AsString()
                              : item1.AsMap().at("name").AsString() < item2.AsMap().at("name").AsString();
                });

    auto end = std::find_if(input_requests_.begin(),
                            input_requests_.end(),
                            [](const auto& item){
                    return item.AsMap().at("type").AsString() == "Bus";
                });

    for(auto i = input_requests_.begin(); i != end; ++i) {
        catalog.AddStop(i->AsMap().at("name"s).AsString(),
                        {i->AsMap().at("latitude").AsDouble(),
                        i->AsMap().at("longitude").AsDouble()});
    }
}

void JsonReader::SetBuses(TransportCatalogue& catalog) {
    auto begin = std::find_if(input_requests_.begin(),
                              input_requests_.end(),
                              [](const auto& item){
                    return item.AsMap().at("type").AsString() == "Bus";
                });

    for(auto i = begin; i != input_requests_.end(); ++i) {
        std::deque<std::string> data;

        for(const auto& item : i->AsMap().at("stops"s).AsArray()) {
            data.push_back(item.AsString());
        }

        std::string last_stop = data[data.size()-1];

        if(!i->AsMap().at("is_roundtrip"s).AsBool()) {
            for(auto i = data.end() - 2; i != data.begin(); --i) {
                data.push_back(*i);
            }
            data.push_back(data[0]);
        }

        catalog.AddBus(i->AsMap().at("name"s).AsString(), data, i->AsMap().at("is_roundtrip"s).AsBool(), last_stop);
    }
}

void JsonReader::SetDistances(TransportCatalogue& catalog) {

    auto end = std::find_if(input_requests_.begin(),
                            input_requests_.end(),
                            [](const auto& item){
                    return item.AsMap().at("type").AsString() == "Bus";
                });

    for(auto item = input_requests_.begin(); item != end; ++item) {
        auto stop_distances = item->AsMap().at("road_distances"s).AsMap();
        for(const auto& i : stop_distances) {
            catalog.GetStopsDistances().insert({{catalog.GetAllStops().at(item->AsMap().at("name"s).AsString()),
                                 catalog.GetAllStops().at(i.first)},
                                i.second.AsInt()});
        }
    }
}

json::Dict JsonReader::GetMap(const MapRenderer& map, const json::Dict& request) {
    json::Dict result;
    result.insert({"map"s, map.GetMap()});
    result.insert({"request_id", request.at("id").AsInt()});
    return result;
}

double JsonReader::GetCurvature(const domain::Bus* bus, int real_distance) {
    double distance = 0;

    for(size_t i = 1; i < bus->stops_.size(); ++i){
        distance += ComputeDistance(
                    bus->stops_[i-1]->coords_,
                    bus->stops_[i]->coords_);
    }

    return static_cast<double>(real_distance) / distance;
}

json::Dict JsonReader::GetBusInfo(TransportCatalogue& catalog,
                                  const json::Dict& request) {
    json::Dict result;
    domain::Bus* bus = catalog.GetBusInfo(request.at("name"s).AsString());

    if(bus) {
        double distance = 0;
        for(size_t i = 1; i < bus->stops_.size(); ++i) {
            distance += catalog.GetDistance(bus->stops_[i-1], bus->stops_[i]);
        }

        result.insert({"curvature", GetCurvature(bus, distance)});
        result.insert({"route_length", distance});
        result.insert({"stop_count", static_cast<int>(bus->stops_.size())});

        result.insert({"unique_stop_count", TransportCatalogue::GetUniqueStopsCount(bus->stops_)});
    } else {
        result.insert({"error_message"s, "not found"s});
    }

    result.insert({"request_id", request.at("id").AsInt()});

    return result;
}

json::Dict JsonReader::GetStopInfo(TransportCatalogue& catalog,
                                   const json::Dict& request) {
    json::Dict result;
    std::optional<std::set<std::string>> stop_buses = catalog.GetStopInfo(request.at("name"s).AsString());

    if(stop_buses) {
        json::Array data(stop_buses->size());
        std::copy(stop_buses->begin(),
                  stop_buses->end(),
                  data.begin()
                    );
        result.insert({"buses"s, data});
    } else {
        result.insert({"error_message"s, "not found"s});
    }

    result.insert({"request_id", request.at("id").AsInt()});

    return result;
}

void JsonReader::SetUnderLayerColor(renderer::MapRenderer& render) {
    auto color = render_settings_.at("underlayer_color"s);
    auto color_node = color.GetNode();
    if(std::get_if<std::string>(&color_node)) {
        render.SetUnderLayerColor(color.AsString());
    } else if(std::get_if<Array>(&color_node)) {
        auto arr = color.AsArray();
        if(arr.size() == 3) {
            svg::Rgb col(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
            render.SetUnderLayerColor(col);
        } else if(arr.size() == 4) {
            svg::Rgba col(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
            render.SetUnderLayerColor(col);
        }
    }
}

void JsonReader::SetColorPalette(renderer::MapRenderer& render) {
    std::vector<svg::Color> palette;
    Array color_palette = render_settings_.at("color_palette"s).AsArray();

    for(const auto& item : color_palette) {
        auto color_node = item.GetNode();

        if(std::get_if<std::string>(&color_node)) {
            palette.push_back(item.AsString());
        } else if(std::get_if<Array>(&color_node)) {
           auto arr = item.AsArray();
           if(arr.size() == 3) {
               svg::Rgb col(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
               palette.push_back(col);
           } else if(arr.size() == 4) {
               svg::Rgba col(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
               palette.push_back(col);
           }
        }
    }

    render.SetColorPalette(palette);
}

void JsonReader::SetSetRenderSettings(renderer::MapRenderer& render) {
    if(json_data_.find("render_settings"s) == json_data_.end()) {
        return;
    }

    render_settings_ = json_data_.at("render_settings"s).AsMap();

    render.SetWidth(render_settings_.at("width"s).AsDouble());
    render.SetHeight(render_settings_.at("height"s).AsDouble());
    render.SetBusFontSize(render_settings_.at("bus_label_font_size"s).AsInt());
    render.SetLineWidth(render_settings_.at("line_width"s).AsDouble());

    render.SetPadding(render_settings_.at("padding"s).AsDouble());
    render.SetStopFontSize(render_settings_.at("stop_label_font_size"s).AsInt());
    render.SetUnderlayerWidth(render_settings_.at("underlayer_width"s).AsDouble());
    render.StopRadius(render_settings_.at("stop_radius"s).AsDouble());

    auto stop_offset_arr = render_settings_.at("stop_label_offset"s).AsArray();
    if(stop_offset_arr.size() == 2) {
        render.SetStopLabelOffset({stop_offset_arr[0].AsDouble(), stop_offset_arr[1].AsDouble()});
    }

    auto bus_offset_arr = render_settings_.at("bus_label_offset"s).AsArray();
    if(bus_offset_arr.size() == 2) {
        render.SetBusLabelOffset({bus_offset_arr[0].AsDouble(), bus_offset_arr[1].AsDouble()});
    }

    SetUnderLayerColor(render);
    SetColorPalette(render);
}

