#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <variant>

namespace svg {

using namespace std::literals;

    struct Rgb {
        Rgb(uint32_t r, uint32_t g, uint32_t b)
            : red(static_cast<uint8_t>(r))
            , green(static_cast<uint8_t>(g))
            , blue(static_cast<uint8_t>(b)) {}
        uint8_t red{};
        uint8_t green{};
        uint8_t blue{};
    };

    struct Rgba : Rgb {
        Rgba(uint32_t r, uint32_t g, uint32_t b, double o)
            : Rgb(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b))
            , opacity(o) {}
        double opacity{ 1.0 };
    };

    std::ostream& operator<<(std::ostream& out, const Rgb& color);
    std::ostream& operator<<(std::ostream& out, const Rgba& color);
    std::ostream& operator<<(std::ostream& out, const std::monostate);

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    template <typename MONO, typename STR, typename RGB, typename RGBA>
    std::ostream& operator<<(std::ostream& out, std::variant<MONO, STR, RGB, RGBA> const& color) {
        std::visit([&](auto&& arg) { out << arg; }, color);
        return out;
    }

    // Объявив в заголовочном файле константу со спецификатором inline,
    // мы сделаем так, что она будет одной на все единицы трансляции,
    // которые подключают этот заголовок.
    // В противном случае каждая единица трансляции будет использовать свою копию этой константы
    inline const Color NoneColor{ "none" };

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

    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap);
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join);

    struct Point {
        Point() = default;
        Point(double first, double second)
            : x(first)
            , y(second) {
        }
        double x{};
        double y{};
    };

    template <typename Owner>
    class PathProps {
    public:
        //задаёт значение свойства fill — цвет заливки.
        //По умолчанию свойство не выводится.
        Owner& SetFillColor(Color color) {
            fill_color_ = std::move(color);
            return AsOwner();
        }
        //задаёт значение свойства stroke — цвет контура.
        //По умолчанию свойство не выводится.
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = std::move(color);
            return AsOwner();
        }
        //задаёт значение свойства stroke-width — толщину линии. 
        //По умолчанию свойство не выводится.
        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }
        //задаёт значение свойства stroke - linecap — тип формы конца линии.
        //По умолчанию свойство не выводится.
        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = std::move(line_cap);
            return AsOwner();
        }
        //задаёт значение свойства stroke-linejoin — тип формы соединения линий. 
        //По умолчанию свойство не выводится.
        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = std::move(line_join);
            return AsOwner();
        }

    protected:
        virtual ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const {

            if (fill_color_) {
                out << " fill=\""sv << *fill_color_ << "\""sv;
            }
            if (stroke_color_) {
                out << " stroke=\""sv << *stroke_color_ << "\""sv;
            }
            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }
            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }
            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }
        }

    private:
        Owner& AsOwner() {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_line_cap_;
        std::optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(std::ostream& output)
            : out(output) {
        }

        RenderContext(std::ostream& output, int indenting_step, int indenting = 0)
            : out(output)
            , indent_step(indenting_step)
            , indent(indenting) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step{};
        int indent{};
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
        Circle(Point center, double radius);
        Circle() = default;
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

        /*
         * Прочие методы и данные, необходимые для реализации элемента <polyline>
         */
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

        // Прочие данные и методы, необходимые для реализации элемента <text>
    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_{};
        Point offset_{};
        uint32_t font_size_{ 1 };
        std::string font_family_;
        std::string font_weight_;
        std::string data_;
    };

    class ObjectContainer {
    public:
        /*
         Метод Add добавляет в svg-документ любой объект-наследник svg::Object.
         Пример использования:
         Document doc;
         doc.Add(Circle().SetCenter({20, 30}).SetRadius(15));
         Чтобы метод Document::Add мог принимать
         произвольные объекты-наследники класса Object по значению,
         сделайте этот метод шаблонным.
         Затем переместите переданное значение в новый объект того же типа в куче,
         используя make_unique:
        */
        template <typename Obj>
        void Add(Obj obj) {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual ~ObjectContainer() {} //виртуальный деструктор т.к. у класса есть виртуальный метод.

    protected:
        // Добавляет в svg-документ объект-наследник svg::Object
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

        // Контейнер, хранящий указатели на объекты
        std::vector<std::unique_ptr<Object>> ptr_objects_;

    };

    class Document : public ObjectContainer {
    public:
        // Выводит в ostream svg-представление документа
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        Document& AddDocument(Document&& doc);
        void Render(std::ostream& out) const;
    };

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() {}
    };
}  // namespace svg
