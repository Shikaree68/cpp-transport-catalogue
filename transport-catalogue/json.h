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
    //using Value = ;
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

    class Node : public std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;
        Node(Value& v) : Value(std::move(v)) {}
        Node() : Value(nullptr) {}
        Node(Value value) : variant(std::move(value)) {}

        bool IsNull() const;
        bool IsArray() const;
        bool IsDict() const;
        bool IsBool() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsDict() const;
        const Value& GetValue() const;
        Value& GetValue();

    };

    bool operator==(const Node& ln, const Node& rn);
    bool operator!=(const Node& ln, const Node& rn);

    void PrintNode(const Node& node, std::ostream& out);

    //// Контекст вывода, хранит ссылку на поток вывода и текущий отсуп
    //struct PrintContext {
    //    std::ostream& out;
    //    int indent_step = 4;
    //    int indent = 0;

    //    void PrintIndent() const {
    //        for (int i = 0; i < indent; ++i) {
    //            out.put(' ');
    //        }
    //    }
    //    // Возвращает новый контекст вывода с увеличенным смещением
    //    PrintContext Indented() const {
    //        return { out, indent_step, indent_step + indent };
    //    }


    //    PrintContext& operator=(const PrintContext& other) {
    //        indent = other.indent;
    //        return *this;
    //    }
    //};


    struct VisitPrinter {
        std::ostream& out;
        void operator()(bool value);
        void operator()(const Array& array);
        void operator()(const Dict& dict);
        void operator()(const std::string& str);
        void operator()(std::nullptr_t);
        template <typename Value>
        void operator()(const Value& value) {
            out << value;
        }
    };
    void PrintValue(Node::Value v, std::ostream& out);

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
