#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/align.hpp>


Widget* Align_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Align_Widget>(*this);
}



void Align_Widget::match(const Align_Def& def) {
    this->child       = this->reconcile(this->child, def.child);
    this->align_point = def.align_point;

    this->mark_for_layout();
}

Bool Align_Widget::on_try_match(Def* def) {
    return try_match_t<Align_Def>(this, def);
}


void Align_Widget::on_layout(Box_Constraints constraints) {
    // TODO: once we have sizing biases.
    //Single_Child_Widget::on_layout(constraints);
    this->child->layout(constraints);
    this->size = constraints.max;
    this->child->position = round(this->align_point * (this->size - this->child->size));
}

