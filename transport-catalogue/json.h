#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <cassert>
#include <sstream>

namespace json {

using namespace std::literals;

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    Node LoadNode(std::istream& input);
    Node LoadNull(std::istream& input);
    Node LoadArray(std::istream& input);
    Node LoadDict(std::istream& input);
    Node LoadBool(std::istream& input);
    Number LoadNumber(std::istream& input);
    // Считывает содержимое строкового литерала JSON-документа
    // Функцию следует использовать после считывания открывающего символа ":
    std::string LoadString(std::istream& input);

    class Node {
    public:
        Node(std::nullptr_t) : value_(nullptr) {}
        Node(Array array) : value_(std::move(array)) {}
        Node(Dict map) : value_(std::move(map)) {}
        Node(bool val) : value_(val) {}
        Node(int value) : value_(value) {}
        Node(double value) : value_(value) {}
        Node(std::string value) : value_(std::move(value)) {}
        Node(Value& v) : value_(std::move(v)) {}
        Node() : value_(nullptr) {}

        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        int& AsInt();
        double AsDouble();
        std::string& AsString();
        bool& AsBool();
        Array& AsArray();
        Dict& AsMap();
        const Value& GetValue() const; 

    private:
        Value value_;
    };

    bool operator==(const Node& ln, const Node& rn);
    bool operator!=(const Node& ln, const Node& rn);

    void PrintNode(const Node& node, std::ostream& out);

    // Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    /*
    struct PrintContext {
        std::ostream& out;
        int indent_step = 4;
        int indent = 0;

        void PrintIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        // Возвращает новый контекст вывода с увеличенным смещением
        PrintContext Indented() const {
            return { out, indent_step, indent_step + indent };
        }
    };
    */
    //int, double, string, bool
    template <typename Value>
    void PrintValue(const Value& value, std::ostream& out) {
        if (std::is_same<bool, Value>::value) {
            if (value) {
                out << "true"sv;
            }
            else out << "false"sv;
        }
        else {
            out << value;
        }
    }
    void PrintValue(const std::string& str, std::ostream& out);
    void PrintValue(const Array& array, std::ostream& out);
    void PrintValue(const Dict& dict, std::ostream& out);
    // Перегрузка функции PrintValue для вывода значений null
    void PrintValue(std::nullptr_t, std::ostream& out);

    class Document {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;
    private:
        Node root_;
    };

    bool operator==(const Document& ld, const Document& rd);
    bool operator!=(const Document& ld, const Document& rd);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json
