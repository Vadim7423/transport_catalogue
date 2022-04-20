#include <algorithm>

#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << " />"sv;
}

// ---------- Polyline ------------------

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    bool flag = false;
    for(const auto& item : points_){
        if(flag) {
            out << " ";
        }
        out << item.x << ","s << item.y;
        if(!flag) {
            flag = true;
        }
    }
    out << "\""sv;
    RenderAttrs(out);
    out << " />"sv;
}

// ---------- Text ------------------

std::unordered_map<char, std::string> GetMap() {
    using namespace std;
    std::unordered_map<char, std::string> replace_symbols = {
        {'"', "&quot;"s},
        {'\'', "&apos;"s},
        {'<', "&lt;"s},
        {'>', "&gt;"s},
        {'&', "&amp;"s}
    };

    return replace_symbols;
}

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}

// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(std::string data) {
    data_.clear();
    std::unordered_map<char, std::string> tmp_map = GetMap();

    std::for_each(
                data.begin(),
                data.end(),
                [&](char item){
                    if(tmp_map.find(item) != tmp_map.end()) {
                        for(char i : tmp_map.at(item)) {
                            data_ += i;
                        }
                    } else {
                        data_ += item;
                    }
                }
             );

    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text x=\""sv << position_.x << "\" y=\""sv << position_.y << "\""sv;
    out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
    out << " font-size=\""sv << font_size_ << "\""sv;

    if(!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }

    if(!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }

    RenderAttrs(out);
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

// ---------- Document ------------------

std::vector<std::unique_ptr<Object>>& ObjectContainer::GetObjects() {
    return objects_;
}

const std::vector<std::unique_ptr<Object>>& ObjectContainer::GetObjects() const {
    return objects_;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    GetObjects().emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

    for(const auto& item : GetObjects()) {
        item->Render(out);
    }

    out << "</svg>"sv;
}

}  // namespace svg

