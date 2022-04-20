#pragma once

#include <cstdint>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <unordered_map>
#include <variant>
#include <iomanip>      // std::setprecision

namespace svg {

struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b)
        : red(r), green(g), blue(b)
    {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
        : red(r), green(g), blue(b), opacity(a)
    {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity  = 1;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

inline std::ostream& operator<<(std::ostream& output, Rgb rgb) {
    using namespace std::string_literals;

    output << "rgb("s;
    output << +rgb.red << ","s;
    output << +rgb.green << ","s;
    output << +rgb.blue << ")"s;

    return output;
}

inline std::ostream& operator<<(std::ostream& output, Rgba rgba) {
    using namespace std::string_literals;

    output << "rgba("s;
    output << +rgba.red << ","s;
    output << +rgba.green << ","s;
    output << +rgba.blue << ","s;
    output << rgba.opacity  << ")"s;

    return output;
}

struct SolutionPrinter {
    std::ostream& out;

    void operator()(std::monostate) const {
        return;
    }

    void operator()(std::string str) const {
        out << str;
    }

    void operator()(Rgb rgb) const {
        out << rgb;
    }

    void operator()(Rgba rgba) const {
        out << rgba;
    }
};

inline const Color NoneColor{"none"};

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

inline std::ostream& operator<<(std::ostream& output, StrokeLineCap line_cap) {
    using namespace std::string_literals;

    if(line_cap == StrokeLineCap::BUTT) {
        output << "butt"s;
    } else if(line_cap == StrokeLineCap::ROUND) {
        output << "round"s;
    } else if(line_cap == StrokeLineCap::SQUARE) {
        output << "square"s;
    }

    return output;
}

inline std::ostream& operator<<(std::ostream& output, StrokeLineJoin line_join) {
    using namespace std::string_literals;

    if(line_join == StrokeLineJoin::ARCS) {
        output << "arcs"s;
    } else if(line_join == StrokeLineJoin::BEVEL) {
        output << "bevel"s;
    } else if(line_join == StrokeLineJoin::MITER) {
        output << "miter"s;
    } else if(line_join == StrokeLineJoin::MITER_CLIP) {
        output << "miter-clip"s;
    } else if(line_join == StrokeLineJoin::ROUND) {
        output << "round"s;
    }

    return output;
}

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }

    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = std::move(line_join);
        return AsOwner();
    }

    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = std::move(line_cap);
        return AsOwner();
    }

    Owner& SetStrokeWidth(double width) {
        stroke_width_ = std::move(width);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        std::ostringstream strm;
        visit(SolutionPrinter{strm}, fill_color_);
        if(!strm.str().empty()) {
            out << " fill=\""sv << strm.str() << "\""sv;
        }

        std::ostringstream strm2;
        visit(SolutionPrinter{strm2}, stroke_color_);
        if(!strm2.str().empty()) {
            out << " stroke=\""sv << strm2.str() << "\""sv;
        }

        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }

        if (line_cap_) {
            out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
        }

        if (line_join_) {
            out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    Color fill_color_;
    Color stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point position_;
    Point offset_;
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer {
public:
    virtual void AddPtr(std::unique_ptr<Object>&&) = 0;

    template <typename Obj>
    void Add(Obj obj) {
        objects_.emplace_back(std::make_unique<Obj>(std::move(obj)));
    }

protected:
    ~ObjectContainer() = default;
    std::vector<std::unique_ptr<Object>>& GetObjects();
    const std::vector<std::unique_ptr<Object>>& GetObjects() const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};


}  // namespace svg
