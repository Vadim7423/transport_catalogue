#pragma once

#include <string_view>
#include <string>
#include<vector>

#include"geo.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace domain {
struct Stop {
    Stop(std::string_view title, double x, double y);

    std::string title_;
    geo::Coordinates coords_;
};

namespace detail {
    struct StopsHasher {
        size_t operator() (const std::pair<Stop*, Stop*>& stops_pair) const ;

        private:
            std::hash<const void*> stop_hasher_;
    };
}

struct Bus {
    Bus(std::string_view title, const std::vector<Stop*>& list, bool is_round, Stop* last_stop);
    double distance = 0;
    std::string title_;
    std::vector<Stop*> stops_;
    Stop* last_stop_;
    bool is_round_ = false;
};
}
