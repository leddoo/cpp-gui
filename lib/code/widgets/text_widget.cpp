// NOTE(llw): File name is to avoid output file collision with code/text.cpp

#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/text.hpp>


Widget* Text_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Text_Widget>(*this);
}



Text_Widget::~Text_Widget() {
    this->layout.destroy();
}


void Text_Widget::match(const Text_Def& def) {
    this->layout.destroy();
    this->layout.create(def.font_face, def.size, def.string);

    this->color      = def.color;
    this->size       = this->layout.size;
    this->baseline.y = round(def.font_face->ascent(def.size));
    // mark layout as changed.

    this->mark_for_paint();
}

Bool Text_Widget::on_try_match(Def* base_def) {
    return try_match_t<Text_Def>(this, base_def);
}


void Text_Widget::on_paint(ID2D1RenderTarget* target) {
    // ensure layout has text.
    if(this->layout.font_face != nullptr) {
        this->layout.paint(target, this->color);
    }
}

