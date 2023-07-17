#include "json.h"

using namespace std;

namespace json {

    Node LoadNode(istream& input) {
        char c;
        input >> c;
        if (c == ']' || c == '}') {
            throw ParsingError("Failed to read number from stream"s);
        }
        else if (c == '[') {
            return LoadArray(input);
        }
        else if (c == '{') {
            return LoadDict(input);
        }
        else if (c == '"') {
            return LoadString(input);
        }
        else if (c == 'n') {
            input.putback(c);
            return LoadNull(input);
        }
        else if (c == 't' || c == 'f') {
            input.putback(c);
            return LoadBool(input);
        }
        else {
            input.putback(c);
            Number number = LoadNumber(input);
            if (number.index() == 0) {
                return get<int>(number);
            }
            else {
                return get<double>(number);
            }
        }
    }
    Node LoadNull(std::istream& input)
    {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        for (char ch : "null"s) {
            if (it != end && ch == *it) {
                ++it;
                continue;
            }
            else {
                throw ParsingError("Failed to read null from stream"s);
            }
        }
        return nullptr;
    }
    Node LoadArray(std::istream& input) {
        Array result;
        char c;
        for (; input >> c && c != ']';) {
            if (c != ',') {
                input.putback(c);
            }
            result.push_back(LoadNode(input));
        }
        if (!result.size() && c != ']') {
            throw ParsingError("Failed to read Array from stream"s);
        }
        return Node(std::move(result));
    }
    Node LoadDict(std::istream& input) {
        Dict result;
        char c;
        for (; input >> c && c != '}';) {
            if (c == ',') {
                input >> c;
            }

            std::string key = LoadString(input);
            input >> c;
            result.insert({ std::move(key), LoadNode(input) });
        }
        if (!result.size() && c != '}') {
            throw ParsingError("Failed to read Dict from stream"s);
        }
        return Node(std::move(result));
    }
    Node LoadBool(std::istream& input)
    {
        char input_ch;
        input >> input_ch;
        std::string b_value;
        bool result;
        if (input_ch == 't') {
            b_value = "rue"s;
            result = true;
        }
        else {
            b_value = "alse"s;
            result = false;
        }
        for (char ch : b_value) {
            input >> input_ch;
            if (ch != input_ch) {
                throw ParsingError("Failed to read boolean from stream"s);
            }
        }
        return result;
    }
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
        }
        else {
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
                }
                catch (...) {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return std::stod(parsed_num);
        }
        catch (...) {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }
    std::string LoadString(std::istream& input) {
        using namespace std::literals;

        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true) {
            if (it == end) {
                // Поток закончился до того, как встретили закрывающую кавычку?
                throw ParsingError("String parsing error");
            }
            const char ch = *it;
            if (ch == '"') {
                // Встретили закрывающую кавычку
                ++it;
                break;
            }
            else if (ch == '\\') {
                // Встретили начало escape-последовательности
                ++it;
                if (it == end) {
                    // Поток завершился сразу после символа обратной косой черты
                    throw ParsingError("String parsing error");
                }
                const char escaped_char = *(it);
                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char) {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r') {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            ++it;
        }

        return s;
    }

    bool Node::IsNull() const {
        if (index() == 0) return true;
        return false;
    }
    bool Node::IsArray() const {
        if (index() == 1) return true;
        return false;
    }
    bool Node::IsDict() const {
        if (index() == 2) return true;
        return false;
    }
    bool Node::IsBool() const {
        if (index() == 3) return true;
        return false;
    }
    bool Node::IsInt() const {
        if (index() == 4) return true;
        return false;
    }
    bool Node::IsDouble() const {
        if (index() == 5 || index() == 4) return true;
        return false;
    }
    bool Node::IsPureDouble() const {
        if (index() == 5) {
            return true;
        }
        return false;
    }
    bool Node::IsString() const {
        if (index() == 6) return true;
        return false;
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return get<int>(*this);
        }
        throw std::logic_error("Not int or double");
    }
    double Node::AsDouble() const {
        if (IsInt()) {
            return get<int>(*this);
        }
        else if (IsDouble()) {
            return get<double>(*this);
        }
        throw std::logic_error("Not int or double");
    }
    const std::string& Node::AsString() const {
        if (IsString()) {
            return get<std::string>(*this);
        }
        throw std::logic_error("Not string"s);
    }
    bool Node::AsBool() const {
        if (IsBool()) {
            return get<bool>(*this);
        }
        throw std::logic_error("Not bool");
    }
    const Array& Node::AsArray() const {
        if (IsArray()) {
            return get<Array>(*this);
        }
        throw std::logic_error("Not array"s);
    }
    const Dict& Node::AsDict() const {
        if (IsDict()) {
            return get<Dict>(*this);
        }
        throw std::logic_error("Not dict!!!"s);
    }

    const Node::Value& Node::GetValue() const { return *this; }
    Node::Value& Node::GetValue() { return *this; }

    bool operator==(const Node& ln, const Node& rn)
    {
        return (ln.GetValue() == rn.GetValue());
    }
    bool operator!=(const Node& ln, const Node& rn)
    {
        return !(ln == rn);
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(
            [&out](const auto& value) { PrintValue(value, out); },
            node.GetValue());
    }

    void VisitPrinter::operator()(bool value) {
        if (value) {
            out << "true"sv;
        }
        else out << "false"sv;
    }

    void VisitPrinter::operator()(const Array& array) {
        out << '[';
        bool first{ true };
        for (const Node& node : array) {
            if (first) {
                PrintNode(node, out);
                first = false;
            }
            else {
                out << ',';
                PrintNode(node, out);
            }
        }
        out << ']';
    }
    void VisitPrinter::operator()(const std::string& str) {
        using namespace std::literals;

        out << '\"';
        for (auto it = str.begin(); it != str.end(); ++it) {
            char symb{ *it };
            if (symb == '\"') {
                out << "\\\""sv;
            }
            else if (symb == '\\') {
                out << "\\\\"sv;
            }
            else if (symb == '\n') {
                out << "\\n"sv;
            }
            else if (symb == '\t') {
                out << '\t';
            }
            else if (symb == '\r') {
                out << "\\r"sv;
            }
            else {
                out << symb;
            }
        }
        out << '\"';
    }
    void VisitPrinter::operator()(const Dict& dict) {
        out << "{ "sv;
        bool first{ true };
        for (const auto& [key, node] : dict) {
            if (first) {
                PrintValue(key, out);
                out << ": "sv;
                PrintNode(node, out);
                first = false;
            }
            else {
                out << ", "sv;
                PrintValue(key, out);
                out << ": "sv;
                PrintNode(node, out);
            }

        }
        out << " }"sv;
    }
    void VisitPrinter::operator()(std::nullptr_t) {
        out << "null"sv;
    }

    void PrintValue(Node::Value v, std::ostream& out){
        std::visit(VisitPrinter{ out }, v);
    }

    Document::Document(Node root)
        : root_(std::move(root)) {
    }
    const Node& Document::GetRoot() const {
        return root_;
    }


    bool operator==(const Document& ld, const Document& rd) {
        return ld.GetRoot() == rd.GetRoot();
    }
    bool operator!=(const Document& ld, const Document& rd) {
        return ld.GetRoot() != rd.GetRoot();
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }
    void Print(const Document& doc, std::ostream& out) {
        PrintNode(doc.GetRoot(), out);
    }

}  // namespace json
