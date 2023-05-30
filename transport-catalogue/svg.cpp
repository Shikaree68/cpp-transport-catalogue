#include "svg.h"

namespace svg {

    using namespace std::literals;

    std::ostream& operator<<(std::ostream& out, const Rgb& color)
    {
        out << "rgb("sv << static_cast<int>(color.red) << ',' << static_cast<int>(color.green) << ',' << static_cast<int>(color.blue) << ')';
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const Rgba& color)
    {
        out << "rgba("sv << static_cast<int>(color.red) << ',' << static_cast<int>(color.green) << ',' << static_cast<int>(color.blue) << ',' << color.opacity << ')';
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const std::monostate)
    {
        out << "none"sv;
        return out;
    }


    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:   out << "butt"sv;    break;
        case StrokeLineCap::ROUND:  out << "round"sv;   break;
        case StrokeLineCap::SQUARE: out << "square"sv;  break;
        default: out.setstate(std::ios_base::failbit);
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
        switch (line_join)
        {
        case StrokeLineJoin::ARCS:      out << "arcs"sv;       break;
        case StrokeLineJoin::BEVEL:     out << "bevel"sv;      break;
        case StrokeLineJoin::MITER:     out << "miter"sv;      break;
        case StrokeLineJoin::MITER_CLIP:out << "miter-clip"sv; break;
        case StrokeLineJoin::ROUND:     out << "round"sv;      break;
        default: out.setstate(std::ios_base::failbit);
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle::Circle(Point center, double radius)
        : center_(center)
        , radius_(radius) {
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first{ true };
        for (const svg::Point& point : points_) {
            if (!first) {
                out << ' ';
            }
            else first = false;
            out << point.x << ',' << point.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }
    // ---------- Text ------------------


    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text";
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << font_size_ << "\""sv;

        if (font_family_.size()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_.size()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv << data_ << "</text>";
    }

    // ---------- Other ------------------
    
    //непонятно зачем перезаписывать
    void ObjectContainer::AddPtr(std::unique_ptr<Object>&& obj) {
        ptr_objects_.push_back(std::move(obj));
    }
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        ptr_objects_.push_back(std::move(obj));
    }

    Document& Document::AddDocument(Document&& doc) {
        ptr_objects_.insert(this->ptr_objects_.end()
            , std::make_move_iterator(doc.ptr_objects_.begin())
            , std::make_move_iterator(doc.ptr_objects_.end()) );
        return *this;
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

        RenderContext ctx{ out, 2, 2 };
        for (const auto& obj : ptr_objects_) {
            obj->Render(ctx);
        }
        out << "</svg>"sv;
    }


}  // namespace svg
