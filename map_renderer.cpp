#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

using namespace transport_list;
using namespace domain;

namespace renderer {

template <typename T>
void SetParametr(T& param, T new_param) {
   if(new_param >= 0 && new_param <= 100000) {
       param = new_param;
   }
}

void MapRenderer::SetWidth(double width) {
    SetParametr(width_, width);
}

void MapRenderer::SetHeight(double height) {
    SetParametr(height_, height);
}

void MapRenderer::SetPadding(double padding) {
    double max_size = std::min(width_, height_)/2;
    if(padding >= 0 && padding < max_size) {
        padding_ = padding;
    }
}

void MapRenderer::SetLineWidth(double line_width) {
    SetParametr(line_width_, line_width);
}

void MapRenderer::StopRadius(double stop_radius) {
    SetParametr(stop_radius_, stop_radius);
}

void MapRenderer::SetBusFontSize(int font_size) {
    SetParametr(bus_label_font_size_, font_size);
}

void MapRenderer::SetStopFontSize(int font_size) {
    SetParametr(stop_label_font_size_, font_size);
}

void MapRenderer::SetUnderlayerWidth(double width) {
    SetParametr(underlayer_width_, width);
}

void MapRenderer::SetBusLabelOffset(const std::vector<double>& params) {
    for(auto i : params) {
        if(i > -100000 && i <= 100000) {
            bus_label_offset_.push_back(i);
        }
    }
}

void MapRenderer::SetStopLabelOffset(const std::vector<double>& params) {
    for(auto i : params) {
        if(i > -100000 && i <= 100000) {
            stop_label_offset_.push_back(i);
        }
    }
}

void MapRenderer::SetUnderLayerColor(const svg::Color& color) {
    underlayer_color_ = color;
}

void MapRenderer::SetColorPalette(const ColorArray& color_arr) {
    color_palette_ = color_arr;
}

svg::Polyline MapRenderer::CreateBusLine(const Bus* bus,
                                         const SphereProjector& projector) {
    svg::Polyline polyline;
    for(const auto& stop : bus->stops_) {
        polyline.AddPoint(projector(stop->coords_));
    }
    return polyline;
}

std::vector<double> GetSortLatItems(const std::unordered_map<std::string_view,
                                    Stop*>& stops) {
    std::vector<double> lat_items(stops.size());
    std::transform(stops.begin(),
                   stops.end(),
                   lat_items.begin(),
                   [](const auto& item){
                        return item.second->coords_.lat;
                    }
                );

    std::sort(lat_items.begin(), lat_items.end());
    return lat_items;
}

std::vector<double> GetSortLonItems(const std::unordered_map<std::string_view,
                                    Stop*>& stops) {
    std::vector<double> lon_items(stops.size());
    std::transform(stops.begin(),
                   stops.end(),
                   lon_items.begin(),
                   [](const auto& item){
                        return item.second->coords_.lng;
                    }
                );

    std::sort(lon_items.begin(), lon_items.end());
    return lon_items;
}

svg::Text MapRenderer::GetUnderlayerTextBus(const Bus* bus,
                                         const Stop* stop,
                                            const SphereProjector& projector) {
    using namespace std::string_literals;
    return svg::Text()
            .SetData(bus->title_)
            .SetFillColor(underlayer_color_)
            .SetStrokeColor(underlayer_color_)
            .SetStrokeWidth(underlayer_width_)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetPosition(projector(stop->coords_))
            .SetOffset({bus_label_offset_[0], bus_label_offset_[1]})
            .SetFontSize(bus_label_font_size_)
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s);
}

svg::Text MapRenderer::GetTextBus(const Bus* bus,
                  const Stop* stop, size_t color_count,
                                  const SphereProjector& projector) {
    using namespace std::string_literals;
    return svg::Text()
            .SetFontSize(bus_label_font_size_)
            .SetData(bus->title_)
            .SetPosition(projector(stop->coords_))
            .SetOffset({bus_label_offset_[0], bus_label_offset_[1]})
            .SetFontFamily("Verdana"s)
            .SetFontWeight("bold"s)
            .SetFillColor(color_palette_[color_count]);
}

svg::Text MapRenderer::GetUnderlayerTextStop(const Stop* stop,
                                             const SphereProjector& projector) {
    using namespace std::string_literals;
    return svg::Text()
            .SetFontSize(stop_label_font_size_)
            .SetData(stop->title_)
            .SetPosition(projector(stop->coords_))
            .SetOffset({stop_label_offset_[0], stop_label_offset_[1]})
            .SetFontFamily("Verdana"s)
            .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
            .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
            .SetStrokeWidth(underlayer_width_)
            .SetStrokeColor(underlayer_color_)
            .SetFillColor(underlayer_color_);
}

svg::Text MapRenderer::GetTextStop(const Stop* stop,
                                   const SphereProjector& projector) {
    using namespace std::string_literals;
    return svg::Text()
            .SetFontSize(stop_label_font_size_)
            .SetData(stop->title_)
            .SetPosition(projector(stop->coords_))
            .SetOffset({stop_label_offset_[0], stop_label_offset_[1]})
            .SetFontFamily("Verdana"s)
            .SetFillColor("black"s);
}

void MapRenderer::SetBuses(const std::map<std::string_view, Bus*> buses,
                           const SphereProjector& projector) {
    using namespace std::string_literals;
    size_t color_count = 0;
    for(const auto& bus : buses) {
        if(bus.second->stops_.empty()) {
            continue;
        }

        map_.Add(CreateBusLine(bus.second, projector)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetFillColor("none"s)
                .SetStrokeColor(color_palette_[color_count++])
                .SetStrokeWidth(line_width_));

        if(color_count == color_palette_.size()) {
            color_count = 0;
        }
    }
}

void MapRenderer::SetBusesLabel(const std::map<std::string_view, Bus*> buses,
                                const SphereProjector& projector) {
    size_t color_count = 0;
    for(const auto& bus : buses) {
        if(bus.second->stops_.empty()) {
            continue;
        }

        auto stop_start = *(bus.second->stops_.begin());
        map_.Add(GetUnderlayerTextBus(bus.second, stop_start, projector));
        map_.Add(GetTextBus(bus.second, stop_start, color_count, projector));

        if(!bus.second->is_round_
                && (stop_start->title_ != bus.second->last_stop_->title_)) {
            map_.Add(GetUnderlayerTextBus(bus.second, bus.second->last_stop_, projector));
            map_.Add(GetTextBus(bus.second, bus.second->last_stop_, color_count, projector));
        }

        ++color_count;
        if(color_count >= color_palette_.size()) {
            color_count = 0;
        }
    }
}

void MapRenderer::SetBusesStops(const std::set<Stop*> stops,
                                const SphereProjector& projector) {
    using namespace std::string_literals;

    for(const auto& stop : stops) {
        map_.Add(svg::Circle()
                .SetCenter(projector(stop->coords_))
                .SetRadius(stop_radius_)
                .SetFillColor("white"s));
    }
}

void MapRenderer::SetBusesStopsLabel(const std::set<Stop*> stops,
                                     const SphereProjector& projector) {

    for(const auto& stop : stops) {
        map_.Add(GetUnderlayerTextStop(stop, projector));
        map_.Add(GetTextStop(stop, projector));
    }
}

void MapRenderer::SetMap(const transport_list::TransportCatalogue& catalog) {
    using namespace std::string_literals;

    std::map<std::string_view, Bus*> buses;

    std::for_each(catalog.GetAllBuses().begin(),
              catalog.GetAllBuses().end(),
              [&] (const auto& item) {
                    buses.insert(item);
                }
                );

    std::vector<geo::Coordinates> stops_coords;
    std::set<Stop*> stops;

    for(const auto& bus : catalog.GetAllBuses()) {
        for(const auto& stop : bus.second->stops_) {
            stops.insert(stop);
            stops_coords.push_back(stop->coords_);
        }
    }

    SphereProjector projector(stops_coords.begin(), stops_coords.end(), width_, height_, padding_);

    SetBuses(buses, projector);

    SetBusesLabel(buses, projector);

    SetBusesStops(stops, projector);

    SetBusesStopsLabel(stops, projector);

   // map_.Render(std::cout);

}

void MapRenderer::RenderMap() const {
    map_.Render(std::cout);
}

std::string MapRenderer::GetMap() const {
    std::ostringstream strm;
    map_.Render(strm);
    std::string str;

    for(char c : strm.str()) {
        if(c == '"') {
           str += '\\';
           str += c;
        } else if(c == '\n') {
           str += '\\';
           str += 'n';
        }  else {
           str += c;
        }
    }

    return str;
}

}
