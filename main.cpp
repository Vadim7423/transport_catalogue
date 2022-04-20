#include <fstream>
#include <sstream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"

using namespace std::string_literals;
using namespace transport_list;
using namespace renderer;

json::Document LoadJSON(const std::string& s) {
    std::istringstream strm(s);
    return json::Load(strm);
}

int main() {
    JsonReader json_reader;
    TransportCatalogue catalog;
    MapRenderer render;
   // RequestHandler request(catalog, render);
    std::ifstream in("E:\\VADIM\\Qt\\practicum_5_14_1_transport_catalogue_visualisation\\write3.json");

    if (in.is_open())
    {
        json_reader.SetData(catalog, render, in);
    }

  //  json_reader.SetData(catalog, render, std::cin);

    in.close();

    render.SetMap(catalog);
    json_reader.GetData(catalog, render);


    /*
     * Примерная структура программы:
     *
     * Считать JSON из stdin
     * Построить на его основе JSON базу данных транспортного справочника
     * Выполнить запросы к справочнику, находящиеся в массиве "stat_requests", построив JSON-массив
     * с ответами.
     * Вывести в stdout ответы в виде JSON
     */
}
