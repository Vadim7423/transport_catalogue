#pragma once

#include <variant>
#include <map>
#include <algorithm>

#include "transport_catalogue.h"
#include "svg.h"
#include "domain.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */
namespace renderer {

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                    double max_height, double padding)
        : padding_(padding) {
        if (points_begin == points_end) {
            return;
        }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lng < rhs.lng;
              });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lat < rhs.lat;
              });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};


using ColorArray = std::vector<svg::Color>;

class MapRenderer {
public:
    void SetWidth(double width);
    void SetHeight(double height);
    void SetPadding(double padding);
    void SetLineWidth(double line_width);
    void StopRadius(double stop_radius);
    void SetUnderlayerWidth(double width);

    void SetBusFontSize(int font_size);
    void SetStopFontSize(int font_size);

    void SetBusLabelOffset(const std::vector<double>& params);
    void SetStopLabelOffset(const std::vector<double>& params);

    void SetUnderLayerColor(const svg::Color& color);
    void SetColorPalette(const ColorArray& color_arr);

    void SetMap(const transport_list::TransportCatalogue& catalog);
    std::string GetMap() const;
    void RenderMap() const;

private:
    svg::Document map_;

    double width_ = 0;
    double height_ = 0;
    double padding_ = 0;
    double stop_radius_ = 1;
    double line_width_ = 1;
    double underlayer_width_ = 0;

    int bus_label_font_size_ = 1;
    int stop_label_font_size_ = 1;

    std::vector<double> bus_label_offset_;
    std::vector<double> stop_label_offset_;

    svg::Color underlayer_color_;
    ColorArray color_palette_;

    void SetBuses(const std::map<std::string_view, domain::Bus*> buses,
                  const SphereProjector& projector);
    void SetBusesLabel(const std::map<std::string_view, domain::Bus*> buses,
                       const SphereProjector& projector);
    void SetBusesStops(const std::set<domain::Stop*> stops,
                       const SphereProjector& projector);
    void SetBusesStopsLabel(const std::set<domain::Stop*> stops,
                            const SphereProjector& projector);

    svg::Polyline CreateBusLine(const domain::Bus* bus,
                                const SphereProjector& projector);

    svg::Text GetUnderlayerTextBus(const domain::Bus* bus,
                                const domain::Stop* stop,
                                   const SphereProjector& projector);
    svg::Text GetTextBus(const domain::Bus* bus,
                      const domain::Stop* stop, size_t color_count,
                         const SphereProjector& projector);

    svg::Text GetUnderlayerTextStop(const domain::Stop* stop,
                                    const SphereProjector& projector);
    svg::Text GetTextStop(const domain::Stop* stop,
                          const SphereProjector& projector);
};

}
