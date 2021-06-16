#include <cpp-gui/widgets/single_child.hpp>


Single_Child_Def::~Single_Child_Def() {
    safe_delete(&this->child);
}



Single_Child_Widget::~Single_Child_Widget() {
    this->drop_maybe(this->child);
    this->child = nullptr;
}


void Single_Child_Widget::match(const Single_Child_Def& def) {
    this->child = this->reconcile(this->child, def.child);
    this->mark_for_layout();
}

Bool Single_Child_Widget::on_try_match(Def* def) {
    return try_match_t<Single_Child_Def>(this, def);
}


void Single_Child_Widget::on_layout(Box_Constraints constraints) {
    // todo: sizing bias.
    if(this->child != nullptr) {
        this->child->layout(constraints);
        this->size = this->child->size;
    }
    else {
        this->size = { 0, 0 };
    }
}


void Single_Child_Widget::on_paint(ID2D1RenderTarget* target) {
    if(this->child != nullptr) {
        this->child->paint(target);
    }
}


Bool Single_Child_Widget::visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) {
    UNUSED(point);
    return this->child != nullptr && visitor(this->child);
}

