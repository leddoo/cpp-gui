#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/rounded.hpp>


Widget* Rounded_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Rounded_Widget>(*this);
}



Float32 Rounded_Widget::get_corner_radius(Widget* widget) {
    auto rounded = dynamic_cast<Rounded_Widget*>(widget);
    if(rounded != nullptr) { return rounded->corner_radius; }
    else                   { return 0.0f;                   }
}

Float32 Rounded_Widget::get_effective_corner_radius(Float32 base_radius, V2f size) {
    return min(base_radius, min(size.x/2.0f, size.y/2.0f));
}

Float32 Rounded_Widget::get_effective_corner_radius(Widget* widget) {
    return get_effective_corner_radius(get_corner_radius(widget), widget->size);
}

Float32 Rounded_Widget::get_effective_corner_radius() {
    return get_effective_corner_radius(this->corner_radius, this->size);
}


void Rounded_Widget::match(const Rounded_Def& def) {
    this->corner_radius = def.corner_radius;
    this->mark_for_paint();
    // TODO: update mouse?
}

Bool Rounded_Widget::on_try_match(Def* def) {
    return try_match_t<Rounded_Def>(this, def);
}


Bool Rounded_Widget::on_hit_test(V2f point) {
    if(Widget::on_hit_test(point) == false) {
        return false;
    }

    auto radius = this->get_effective_corner_radius();
    if(radius > 0.0f) {
        auto min = V2f(radius);
        auto max = this->size - V2f(radius);
        auto corner = point;

        if     (point.x < min.x && point.y < min.y) {
            corner = { min.x, min.y };
        }
        else if(point.x < min.x && point.y > max.y) {
            corner = { min.x, max.y };
        }
        else if(point.x > max.x && point.y < min.y) {
            corner = { max.x, min.y };
        }
        else if(point.x > max.x && point.y > max.y) {
            corner = { max.x, max.y };
        }

        if(length_squared(corner - point) > squared(radius)) {
            return false;
        }
    }

    return true;
}

