#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/multi_child.hpp>


Multi_Child_Def::~Multi_Child_Def() {
    for(auto child : this->children) {
        delete child;
    }
    this->children.clear();
}

Widget* Multi_Child_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Multi_Child_Widget>(*this);
}



Multi_Child_Widget::~Multi_Child_Widget() {
    for(auto child : this->children) {
        this->drop(child);
    }
}


void Multi_Child_Widget::match(const Multi_Child_Def& def) {
    this->children = this->reconcile_list(this->children, def.children);
    this->mark_for_layout();
}

Bool Multi_Child_Widget::on_try_match(Def* def) {
    return try_match_t<Multi_Child_Def>(this, def);
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

