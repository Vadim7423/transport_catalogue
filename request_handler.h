#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include"domain.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

// Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
// с другими подсистемами приложения.
// См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const transport_list::TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    domain::Bus* GetBusStat(const std::string& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::optional<std::set<std::string>> GetBusesByStop(const std::string& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    void RenderMap();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const transport_list::TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};

