#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <optional>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeJson = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node : private NodeJson {
public:
    explicit Node() = default;
    template <typename T>
    Node(T val)
        : node_json_(val)
    {}

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;

    bool IsNull() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;
    bool IsBool() const;
    bool IsArray() const;
    bool IsMap() const;

    NodeJson GetNode() const;

    bool operator==(const Node& rhs) const {
        return node_json_ == rhs.node_json_;
    }

    bool operator!=(const Node& rhs) const {
        return !(node_json_ == rhs.node_json_);
    }

private:
    NodeJson node_json_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    static void StreamUpdForArr(std::ostream& out, const Array& dict);
    static void StreamUpdForDict(std::ostream& out, const Dict& dict);

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
