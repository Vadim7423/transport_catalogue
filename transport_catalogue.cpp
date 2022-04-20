#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>

using namespace domain;

namespace transport_list {

    size_t TransportCatalogue::GetDistance(Stop* from, Stop* to) const {
        size_t distance = 0;
        if(stops_distances_.find({from, to}) == stops_distances_.end()) {
            if(stops_distances_.find({to, from}) == stops_distances_.end()) {
                return distance;
            }
            distance = stops_distances_.at({to, from});
        }else{
            distance = stops_distances_.at({from, to});
        }
       // std::cout << distance << std::endl;
        return distance;
    }

    void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coords) {
        stops_list_.push_back({name, coords.lat, coords.lng});
        Stop* last = &stops_list_[stops_list_.size()-1];
        stops_.insert({last->title_, last});

        if(stop_buses_.find(last) == stop_buses_.end()) {
            stop_buses_.insert({last, {}});
        }
    }

    void TransportCatalogue::AddBus(const std::string& name,
                                    const std::deque<std::string>& stops,
                                    bool is_round, const std::string& last_stop) {
        std::vector<Stop*> loc_stops(stops.size());
        std::transform(
                    stops.begin(),
                    stops.end(),
                    loc_stops.begin(),
                    [&](std::string_view str) {
                       return stops_.at(str);
                    }
                 );
        buses_list_.push_back(Bus(name, loc_stops, is_round, stops_.at(last_stop)));
        Bus* last = &buses_list_[buses_list_.size()-1];

        for(const auto& stop : last->stops_) {
            stop_buses_.at(stop).insert(last->title_);
        }

        buses_.insert({last->title_, last});
    }

    const Stop* TransportCatalogue::SearchStop(const std::string& name) {
        return stops_.at(name);
    }

    const Bus* TransportCatalogue::SearchBus(const std::string& name) {
        return buses_.at(name);
    }

    Bus* TransportCatalogue::GetBusInfo(const std::string& name) const {
        auto it = buses_.find(name);

        if(it == buses_.end()) {
            return nullptr;
        }

        if(it->second->stops_.size() < 2) {
            return buses_.at(name);
        }

        return buses_.at(name);
    }

    std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(const std::string& name) const {
        if(stops_.find(name) == stops_.end()) {
            return std::nullopt;
        }

        return stop_buses_.at(stops_.at(name));
    }

    const std::unordered_map<std::string_view, Stop*>& TransportCatalogue::GetAllStops() const {
        return stops_;
    }

    const std::unordered_map<std::string_view, Bus*>& TransportCatalogue::GetAllBuses() const {
        return buses_;
    }

    std::unordered_map<std::pair<Stop*, Stop*>, size_t, detail::StopsHasher>& TransportCatalogue::GetStopsDistances() {
        return stops_distances_;
    }

    int TransportCatalogue::GetUniqueStopsCount(const std::vector<Stop*>& stops) {
        auto copy_stops = stops;
        std::sort(copy_stops.begin(), copy_stops.end());
        auto last = std::unique(copy_stops.begin(), copy_stops.end());
        return last - copy_stops.begin();
    }
}

