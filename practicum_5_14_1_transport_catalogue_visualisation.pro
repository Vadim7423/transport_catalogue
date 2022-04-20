TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        domain.cpp \
        geo.cpp \
        json.cpp \
        json_reader.cpp \
        main.cpp \
        map_renderer.cpp \
        request_handler.cpp \
        svg.cpp \
        transport_catalogue.cpp

HEADERS += \
    domain.h \
    geo.h \
    json.h \
    json_reader.h \
    map_renderer.h \
    request_handler.h \
    svg.h \
    transport_catalogue.h
