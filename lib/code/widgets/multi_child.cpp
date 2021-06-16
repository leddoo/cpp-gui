#include <cpp-gui/widgets/multi_child.hpp>


Multi_Child_Widget::~Multi_Child_Widget() {
    for(auto child : this->children) {
        this->drop(child);
    }
}


void Multi_Child_Widget::on_paint(ID2D1RenderTarget* target) {
    for(auto child : this->children) {
        child->paint(target);
    }
}


Bool Multi_Child_Widget::visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) {
    UNUSED(point);

    for(auto it = this->children.rbegin(); it != this->children.rend(); ++it) {
        if(visitor(*it)) {
            return true;
        }
    }

    return false;
}

