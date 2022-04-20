#include "domain.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области
 * (domain) вашего приложения и не зависят от транспортного справочника. Например Автобусные
 * маршруты и Остановки.
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

#include"domain.h"

namespace domain {

Stop::Stop(std::string_view title, double x, double y)
    : title_(title), coords_({x,y})
{}

namespace detail {
    size_t StopsHasher::operator() (const std::pair<Stop*, Stop*>& stops_pair) const {
        size_t stop_one = stop_hasher_(static_cast<const void*>(stops_pair.first));
        size_t stop_two = stop_hasher_(static_cast<const void*>(stops_pair.second));
        return stop_one + stop_two*37;
    }
}

Bus::Bus(std::string_view title, const std::vector<Stop*>& list, bool is_round, Stop* last_stop)
    : title_(title), stops_(list), last_stop_(last_stop), is_round_(is_round)
{
}
}
