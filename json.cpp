#include <sstream>
#include <cmath>
#include <unordered_map>
#include <set>
#include<iomanip>

#include "json.h"

using namespace std;

namespace json {

namespace {

using Number = std::variant<int, double>;

Number LoadNumber(std::istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    bool is_close = false;

    if(input.peek() == -1) {
        throw ParsingError("load array error - empty stream"s);
    }

    for (char c; input >> c;) {
        if(c == ']') {
            is_close = true;
            break;
        }

        if (c != ',') {
            input.putback(c);
        }

        auto item = LoadNode(input).GetNode();

        result.push_back(move(item));
    }

    if(!is_close) {
        throw ParsingError("array error - no close symbol"s);
    }

     return Node(move(result));
}
/*
Node LoadInt(istream& input) {
    int result = 0;
    while (isdigit(input.peek())) {
        result *= 10;
        result += input.get() - '0';
    }

    return Node(result);
}*/

Node LoadOther(istream& input) {
    string line;
    std::set<char> symbols = {'}', ']', ' ', -1, ',', 10, 13};

    if(input.peek() == -1) {
        throw ParsingError("load other value  error"s);
    }
    /*
    for(char c; input >> c;) {
        if(symbols.find(c) != symbols.end()) {
            break;
        }
        line += c;
    }*/
    while(symbols.find(input.peek()) == symbols.end()) {
        line += input.get();
    }

    if(line == "null"s) {
        return Node(nullptr);
    }

    if(line == "true"s) {
        return Node(true);
    }

    if(line == "false"s) {
        return Node(false);
    }

    while(symbols.find(line[line.size()-1]) != symbols.end()) {
        line.pop_back();
    }

    istringstream strm(line);
    auto value = LoadNumber(strm);

    if(std::get_if<int>(&value)) {
        return std::get<int>(value);
    }

    if(std::get_if<double>(&value)) {
        return std::get<double>(value);
    }

    return Node(nullptr);
}

Node LoadString(istream& input) {
    static std::unordered_map<char, char> symbols = {
        {'\\', '\\'},
        {'"', '\"'},
        {'n', '\n'},
        {'r', '\r'},
        {'b', '\b'},
        {'f', '\f'},
        {'t', '\t'}
    };

    bool is_end_str = false;

    if(input.peek() == -1) {
        throw ParsingError("load string  error - empty stream"s);
    }

    string line;
    char c;

    while (input.get(c)) {
        if(c == '\\' && symbols.find(input.peek()) != symbols.end()) {
            c = symbols.at(input.get());
        } else if(c == '"') {
            is_end_str = true;
            break;
        }

        line += c;

     }

    if(!is_end_str) {
        throw ParsingError("load string  error - no close symbol"s);
    }

    return Node(move(line));
}

Node LoadDict(istream& input) {
    Dict result;
    bool is_close = false;

    if(input.peek() == -1) {
        throw ParsingError("load dict  error - empty stream"s);
    }

    for (char c; input >> c;) {
        if(c == '}') {
            is_close = true;
            break;
        }

        if (c == ',' || c == '\n') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({move(key), LoadNode(input)});

    }

   if(!is_close) {
      throw ParsingError("dict error - no close symbol"s);
   }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if(c == '}' || c == ']') {
        throw ParsingError("load value  error - invalid first symbol"s);
    } else {
      //  std::cout << c << std::endl;
        input.putback(c);
        return LoadOther(input);
    }
}

}  // namespace

const Array& Node::AsArray() const {
    if(std::get_if<Array>(&node_json_)) {
        return std::get<Array>(node_json_);
    }
    throw std::logic_error("value not array"s);
}

const Dict& Node::AsMap() const {
    if(std::get_if<Dict>(&node_json_)) {
        return std::get<Dict>(node_json_);
    }
    throw std::logic_error("value not dict"s);
}

int Node::AsInt() const {
    if(!std::get_if<int>(&node_json_)) {
        throw std::logic_error("value not int"s);
    }
    return std::get<int>(node_json_);
}

double Node::AsDouble() const {
    if(std::get_if<int>(&node_json_)) {
        return std::get<int>(node_json_)*1.0;
    }

    if(std::get_if<double>(&node_json_)) {
        return std::get<double>(node_json_);
    }

    throw std::logic_error("value not double"s);
}

const string& Node::AsString() const {
    if(std::get_if<std::string>(&node_json_)) {
        return std::get<std::string>(node_json_);
    }
    throw std::logic_error("value not string"s);
}

bool Node::AsBool() const {
    if(std::get_if<bool>(&node_json_)) {
        return std::get<bool>(node_json_);
    }
    throw std::logic_error("value not bool"s);
}

bool Node::IsNull() const {
    if(std::get_if<std::nullptr_t>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsInt() const {
    if(std::get_if<int>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsDouble() const {
    auto double_num = std::get_if<double>(&node_json_);
    auto int_num = std::get_if<int>(&node_json_);

    if(double_num || int_num) {
        return true;
    }
    return false;
}

bool Node::IsPureDouble() const {
    if(std::get_if<double>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsString() const {
    if(std::get_if<std::string>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsBool() const {
    if(std::get_if<bool>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsArray() const {
    if(std::get_if<Array>(&node_json_)) {
        return true;
    }
    return false;
}

bool Node::IsMap() const {
    if(std::get_if<Dict>(&node_json_)) {
        return true;
    }
    return false;
}

NodeJson Node::GetNode() const {
    return node_json_;
}

Document::Document(Node root)
    : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

std::string StrUpd(const std::string& str) {
    std::string new_str = "\""s;
    for(char i : str) {
        if(i == '\\' || i == '"') {
            new_str += '\\';
        }
        new_str += i;
    }

    new_str += "\""s;

    return new_str;
}

void Document::StreamUpdForArr(std::ostream& out, const Array& arr) {
    out << "[\n"s;
    bool flag = false;
    for(const auto& i : arr) {
        if(flag) {
            out << ",\n"s;
        }

        auto item = i.GetNode();
        if(std::get_if<std::string>(&item)) {
            out << "  \""s << i.AsString() << '"';
        } else if(std::get_if<int>(&item)) {
            out << "  "s << i.AsInt();
        } else if(std::get_if<double>(&item)) {
            out << "  "s << i.AsDouble();
        } else if(std::get_if<bool>(&item)) {
            if(i.AsBool()) {
                out << "  "s << "true"s;
            } else {
                out << "  "s << "false"s;
            }
        } else if(std::get_if<Array>(&item)) {
            StreamUpdForArr(out, i.AsArray());
        } else if(std::get_if<Dict>(&item)) {
            StreamUpdForDict(out, i.AsMap());
        } else if(std::get_if<std::nullptr_t>(&item)) {
            out << "  "s << "null"s;
        }

        if(!flag) {
            flag = true;
        }
    }
    out << "]"s;
}

void Document::StreamUpdForDict(std::ostream& out, const Dict& dict) {
    out << "{\n"s;
    bool flag = false;
    for(const auto& i : dict) {
        if(flag) {
            out << ",\n"s;
        }
        out << "  \""s << i.first << "\""s << ":"s;
        auto item = i.second.GetNode();
        if(std::get_if<std::string>(&item)) {
            out << '"' << i.second.AsString() << '"';
        } else if(std::get_if<int>(&item)) {
            out << i.second.AsInt();
        } else if(std::get_if<double>(&item)) {
            /*if(i.second.AsDouble() >= 1000000) {
                 out << std::setprecision(4) << fixed << std::scientific << i.second.AsDouble();
            } else {
                out  << fixed << i.second.AsDouble();
            }*/
            out  << i.second.AsDouble();
        } else if(std::get_if<bool>(&item)) {
            if(i.second.AsBool()) {
                out << "true"s;
            } else {
                out << "false"s;
            }
        } else if(std::get_if<Array>(&item)) {
            StreamUpdForArr(out, i.second.AsArray());
        } else if(std::get_if<Dict>(&item)) {
            StreamUpdForDict(out, i.second.AsMap());
        } else if(std::get_if<std::nullptr_t>(&item)) {
            out << "null"s;
        }

        if(!flag) {
            flag = true;
        }
    }
    out << "}"s;
}

struct SolutionPrinter {
    std::ostream& out;

    void operator()(std::nullptr_t) const {
        out << "null"s;
    }

    void operator()(Array arr) const {
        Document::StreamUpdForArr(out, arr);
    }

    void operator()(Dict dict) const {
        Document::StreamUpdForDict(out, dict);
    }

    void operator()(bool flag) const {
        if(flag) {
            out << "true"s;
        } else {
            out << "false"s;
        }
    }

    void operator()(int item) const {
        out << item;
    }

    void operator()(double item) const {
        out << item;
    }

    void operator()(std::string item) const {
        out << StrUpd(item);
    }
};

void Print(const Document& doc, std::ostream& output) {
    std::ostringstream strm;
    auto root = doc.GetRoot();
    auto json_node = root.GetNode();
    visit(SolutionPrinter{strm}, json_node);

    if(!strm.str().empty()) {
//        std::cout << strm.str() << std::endl;
        output << strm.str();
    }
}


}  // namespace json
