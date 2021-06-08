#include "core.hpp"

void Widget::adopt(Widget* child) {
    assert(child->parent == nullptr);
    child->parent = this;
}

void Widget::drop(Widget* child) {
    if(child->owner == this) { 
        if(child->parent == this || child->parent == nullptr) {
            // owner and parent -> drop.
            child->owner  = nullptr;
            child->parent = nullptr;
            gui->destroy_widget(child);
        }
        else {
            // owner but not parent -> parent becomes owner.
            child->owner = child->parent;
        }
    }
    else if(child->parent == this) {
        // Parent has dropped child owned by someone else.
        // Child will likely be re-parented.
        child->parent = nullptr; 
    }
    else {
        throw Unreachable();
    }
}

Widget* Widget::reconcile(Widget* child, Gui_Node* node) {
    auto widget = dynamic_cast<Widget*>(node);
    auto props  = dynamic_cast<Props*>(node);

    if(widget != nullptr) {
        return this->reconcile_widget(child, widget);
    }
    else if(props != nullptr) {
        return this->reconcile_props(child, props);
    }
    else {
        throw Unimplemented {};
    }
}

Widget* Widget::reconcile_widget(Widget* child, Widget* widget) {
    if(widget != child) {
        this->drop_maybe(child);
        this->adopt(widget);
    }

    return widget;
}

Widget* Widget::reconcile_props(Widget* child, Props* props) {
    auto prop_child = dynamic_cast<Prop_Widget*>(child);
    if(prop_child != nullptr) {
        auto used = prop_child->on_props_changed(props);
        if(used) {
            return prop_child;
        }
    }

    this->drop_maybe(child);

    return gui->inflate_widget(this, props);
}


void Widget::mark_for_layout() {
    gui->request_frame();
}

void Widget::layout(Box_Constraints constraints) {
    this->on_layout(constraints);
}


void Widget::mark_for_paint() {
    gui->request_frame();
}

void Widget::paint(ID2D1RenderTarget* target) {
    auto old_tfx = D2D_MATRIX_3X2_F {};
    target->GetTransform(&old_tfx);
    target->SetTransform(old_tfx * D2D1::Matrix3x2F::Translation(to_d2d_sizef(this->position)));

    this->on_paint(target);

    target->SetTransform(old_tfx);
}



void Single_Child_Widget::on_destroy() {
    this->drop(this->child);
}

void Single_Child_Widget::on_layout(Box_Constraints constraints) {
    this->child->layout(constraints);
    this->size = this->child->size;
}

void Single_Child_Widget::on_paint(ID2D1RenderTarget* target) {
    this->child->paint(target);
}



void Gui::request_frame() {
    if(!this->has_requested_frame) {
        this->request_frame_callback();
        this->has_requested_frame = true;
    }
}

void Gui::create(Gui_Node* root, Void_Callback request_frame) {
    this->request_frame_callback = request_frame;

    this->root_widget.gui    = this;
    this->root_widget.owner  = &this->root_widget;
    this->root_widget.parent = &this->root_widget;
    this->root_widget.child  = this->root_widget.reconcile(nullptr, root);
}

void Gui::destroy() {
    this->root_widget.on_destroy();

    *this = {};
}


void Gui::render_frame(V2f size, ID2D1RenderTarget* target) {
    this->root_widget.layout(Box_Constraints::tight(size));
    this->root_widget.paint(target);
    this->has_requested_frame = false;
}


Widget* Gui::inflate_widget(Widget* parent, Props* props) {
    auto widget = props->create_widget();
    widget->gui    = this;
    widget->owner  = parent;
    widget->parent = parent;
    widget->on_create();

    auto used_props = widget->on_props_changed(props);
    assert(used_props);

    return widget;
}

void Gui::destroy_widget(Widget* widget) {
    assert(widget->owner == nullptr && widget->parent == nullptr);
    widget->on_destroy();
    delete widget;
}

void Gui::destroy_props(Props* props) {
    props->on_destroy();
    delete props;
}

