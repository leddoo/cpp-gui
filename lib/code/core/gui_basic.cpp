#include <cpp-gui/core/gui.hpp>


void Gui::request_frame() {
    if(!this->has_requested_frame) {
        this->request_frame_callback();
        this->has_requested_frame = true;
    }
}



void Gui::create(Def* root_def, Void_Callback request_frame) {
    this->request_frame_callback = request_frame;

    if(root_def != nullptr) {
        this->set_root(root_def);
    }
}


void Gui::destroy() {
    safe_delete(&this->root_widget);
}


void Gui::set_root(Def* def) {
    auto& root = this->root_widget;

    auto temp_parent = Widget();
    temp_parent.gui = this;

    // To make `drop` work in reconcile.
    if(root != nullptr) {
        root->owner  = &temp_parent;
        root->parent = &temp_parent;
    }

    root = temp_parent.reconcile(root, def, Widget::New_Child_Action::none);
    root->owner  = nullptr;
    root->parent = nullptr;
}


void Gui::render_frame(V2f size, ID2D1RenderTarget* target) {
    this->root_widget->layout(Box_Constraints::tight(size));
    this->root_widget->paint(target);
    this->has_requested_frame = false;
}

