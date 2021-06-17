#include <cpp-gui/widgets/solid.hpp>
#include <cpp-gui/widgets/rounded.hpp>
#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/d2d.hpp>


Widget* Solid_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Solid_Widget>(*this);
}



void Solid_Widget::match(const Solid_Def& def) {
    this->fill_color = def.fill_color;
    this->stroke_color = def.stroke_color;
    this->mark_for_paint();
}

Bool Solid_Widget::on_try_match(Def* def) {
    return try_match_t<Solid_Def>(this, def);
}


void Solid_Widget::on_paint(ID2D1RenderTarget* target) {
    auto radius = Rounded_Widget::get_effective_corner_radius(this);

    if(this->fill_color.a > 0.0f) {
        auto brush = (ID2D1SolidColorBrush*)nullptr;
        auto hr = target->CreateSolidColorBrush(to_d2d_colorf(this->fill_color), &brush);
        if(SUCCEEDED(hr)) {
            target->FillRoundedRectangle(
                D2D1::RoundedRect(D2D1::RectF(0, 0, this->size.x, this->size.y), radius, radius),
                brush
            );
            brush->Release();
        }
    }

    if(this->stroke_color.a > 0.0f) {
        auto brush = (ID2D1SolidColorBrush*)nullptr;
        auto hr = target->CreateSolidColorBrush(to_d2d_colorf(this->stroke_color), &brush);
        if(SUCCEEDED(hr)) {
            target->DrawRoundedRectangle(
                D2D1::RoundedRect(D2D1::RectF(0.5f, 0.5f, this->size.x - 0.5f, this->size.y - 0.5f), radius, radius),
                brush
            );
            brush->Release();
        }
    }
}

