#include <unordered_map>

#include "core.hpp"


Def* Def::with_key(Key* key) {
    assert(dynamic_cast<Widget_Def*>(this) == nullptr);
    assert(dynamic_cast<Keyed_Def*>(this) == nullptr);

    return new Keyed_Def(this, key);
}


Widget* Widget_Def::get_widget(Gui* gui) {
    UNUSED(gui);
    throw Unreachable();
}


Keyed_Def::~Keyed_Def() {
    if(this->key != nullptr) {
        delete this->key;
        this->key = nullptr;
    }

    delete this->def;
    this->def = nullptr;
}

Widget* Keyed_Def::get_widget(Gui* gui) {
    UNUSED(gui);
    throw Unreachable();
}



void Widget::adopt(Widget* child) {
    assert(child->parent == nullptr);

    if(child->owner == nullptr) {
        child->owner = this;
    }

    child->parent = this;
}

void Widget::drop(Widget* child) {
    if(child->owner == this) {
        if(child->parent != nullptr && child->parent != this) {
            // other parent -> transfer ownership.
            child->owner = child->parent;
        }
        else {
            // owner and parent -> destroy.
            child->owner  = nullptr;
            child->parent = nullptr;
            gui->destroy_widget(child);
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

void Widget::drop_maybe(Widget* child) {
    if(child != nullptr) {
        this->drop(child);
    }
}


Widget* Widget::reconcile(Widget* old_widget, Def* new_def, Bool adopt) {
    auto widget_def = dynamic_cast<Widget_Def*>(new_def);
    auto keyed_def  = dynamic_cast<Keyed_Def*>(new_def);

    if(widget_def != nullptr) {
        auto new_widget = widget_def->widget;

        if(old_widget != new_widget) {
            this->drop_maybe(old_widget);
            if(adopt) {
                this->adopt(new_widget);
            }
        }

        return new_widget;
    }
    else if(keyed_def != nullptr) {
        auto new_widget = this->reconcile(old_widget, keyed_def->def, adopt);

        std::swap(new_widget->key, keyed_def->key);

        return new_widget;
    }
    else if(old_widget != nullptr && old_widget->on_try_match(new_def)) {
        return old_widget;
    }
    else {
        auto new_widget = new_def->get_widget(gui);

        this->drop_maybe(old_widget);
        if(adopt) {
            this->adopt(new_widget);
        }

        return new_widget;
    }
}

List<Widget*> Widget::reconcile_list(List<Widget*> old_widgets, List<Def*> new_defs, Bool adopt) {
    // NOTE(llw): Populate maps.
    auto widget_to_index = std::unordered_map<Widget*,     Uint>();
    auto key_to_index    = std::unordered_map<Key_Pointer, Uint>();
    for(Uint index = 0; index < old_widgets.size(); index += 1) {
        auto widget = old_widgets[index];

        widget_to_index.insert({ widget, index });

        if(widget->key != nullptr) {
            key_to_index.insert({ Key_Pointer(widget->key), index });
        }
    }

    auto new_widgets = List<Widget*>(new_defs.size());

    // NOTE(llw): Process widget defs and keyed defs.
    for(Uint def_index = 0; def_index < new_defs.size(); def_index += 1) {
        auto new_def = new_defs[def_index];

        auto widget_def = dynamic_cast<Widget_Def*>(new_def);
        auto keyed_def  = dynamic_cast<Keyed_Def*>(new_def);

        if(widget_def != nullptr) {
            // Had this widget previously?
            auto it = widget_to_index.find(widget_def->widget);
            if(it != widget_to_index.end()) {
                // Mark old widget as used.
                auto old_index = it->second;
                auto old_widget = old_widgets[old_index];
                assert(old_widget != nullptr);
                old_widgets[old_index] = nullptr;

                new_widgets[def_index] = old_widget;
            }
        }
        else if(keyed_def != nullptr) {
            // Had this key previously?
            auto it = key_to_index.find(keyed_def->key);
            if(it != key_to_index.end()) {
                // Mark old widget as used.
                auto old_index = it->second;
                auto old_widget = old_widgets[old_index];
                assert(old_widget != nullptr);
                old_widgets[old_index] = nullptr;

                new_widgets[def_index] = this->reconcile(old_widget, new_def, adopt);
            }
        }
    }

    // Process remaining defs.
    auto old_cursor = Uint(0);
    for(Uint def_index = 0; def_index < new_defs.size(); def_index += 1) {
        auto new_def = new_defs[def_index];

        // Def already processed?
        if(new_widgets[def_index] != nullptr) {
            continue;
        }

        // Get next unprocessed old widget.
        auto old_widget = (Widget*)nullptr;
        while(old_cursor < old_widgets.size()) {
            auto& at = old_widgets[old_cursor];
            old_cursor += 1;

            if(at != nullptr) {
                old_widget = at;
                at = nullptr;
                break;
            }
        }

        new_widgets[def_index] = this->reconcile(old_widget, new_def, adopt);
    }

    // Drop remaining old widgets.
    for(; old_cursor < old_widgets.size(); old_cursor += 1) {
        this->drop_maybe(old_widgets[old_cursor]);
    }

    return new_widgets;
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



void Gui::request_frame() {
    if(!this->has_requested_frame) {
        this->request_frame_callback();
        this->has_requested_frame = true;
    }
}

void Gui::create(Def* root_def, Void_Callback request_frame) {
    this->request_frame_callback = request_frame;

    this->root_widget         = new Root_Widget();
    this->root_widget->gui    = this;
    this->root_widget->owner  = this->root_widget;
    this->root_widget->parent = this->root_widget;
    this->root_widget->child  = this->root_widget->reconcile(nullptr, root_def);

    delete root_def;
}

void Gui::destroy() {
    delete this->root_widget;
}


void Gui::render_frame(V2f size, ID2D1RenderTarget* target) {
    this->root_widget->layout(Box_Constraints::tight(size));
    this->root_widget->paint(target);
    this->has_requested_frame = false;
}


void Gui::destroy_widget(Widget* widget) {
    assert(widget->owner == nullptr && widget->parent == nullptr);
    if(widget->key != nullptr) {
        delete widget->key;
    }
    delete widget;
}




Bool Uint_Key::equal_to(const Key* other) const {
    auto other_as_uint_key = dynamic_cast<const Uint_Key*>(other);
    if(other_as_uint_key == nullptr) {
        return false;
    }

    return this->value == other_as_uint_key->value;
}

Uint Uint_Key::hash() const {
    return std::hash<Uint>()(this->value);
}



Single_Child_Widget::~Single_Child_Widget() {
    this->drop(this->child);
    this->child = nullptr;
}

void Single_Child_Widget::on_layout(Box_Constraints constraints) {
    this->child->layout(constraints);
    this->size = this->child->size;
}

void Single_Child_Widget::on_paint(ID2D1RenderTarget* target) {
    this->child->paint(target);
}


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

