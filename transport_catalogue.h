#pragma once

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <optional>

#include "domain.h"

namespace transport_list {

    class TransportCatalogue {
    public:
        void AddStop(const std::string& name, const geo::Coordinates& coords);
        // deque используется потому что потом из массива stops формируется массив указателей
        void AddBus(const std::string& name, const std::deque<std::string>& stops,
                    bool is_round, const std::string& last_stop);

        const domain::Stop* SearchStop(const std::string& name);
        const domain::Bus* SearchBus(const std::string& name);

        domain::Bus* GetBusInfo(const std::string& name) const;
        std::optional<std::set<std::string>> GetStopInfo(const std::string& name) const;

        size_t GetDistance(domain::Stop* from, domain::Stop* to) const;

        static int GetUniqueStopsCount(const std::vector<domain::Stop*>& stops);

        const std::unordered_map<std::string_view, domain::Stop*>& GetAllStops() const;
        const std::unordered_map<std::string_view, domain::Bus*>& GetAllBuses() const;
        std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, size_t, domain::detail::StopsHasher>& GetStopsDistances();

    private:
        std::deque<domain::Stop> stops_list_;
        std::unordered_map<std::string_view, domain::Stop*> stops_;
        std::deque<domain::Bus> buses_list_;
        std::unordered_map<std::string_view, domain::Bus*> buses_;
        std::unordered_map<domain::Stop*, std::set<std::string>> stop_buses_;
        std::unordered_map<std::pair<domain::Stop*, domain::Stop*>, size_t, domain::detail::StopsHasher> stops_distances_;
    };
}



