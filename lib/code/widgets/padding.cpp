#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/padding.hpp>


Widget* Padding_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Padding_Widget>(*this);
}



void Padding_Widget::match(const Padding_Def& def) {
    this->child = this->reconcile(this->child, def.child);
    this->pad_min = def.pad_min;
    this->pad_max = def.pad_max;
    this->mark_for_layout();
}

Bool Padding_Widget::on_try_match(Def* def) {
    return try_match_t<Padding_Def>(this, def);
}


void Padding_Widget::on_layout(Box_Constraints constraints) {
    // todo: constraints.
    this->child->layout(constraints);
    this->child->position = this->pad_min;
    this->size = this->pad_min + this->child->size + this->pad_max;
}

