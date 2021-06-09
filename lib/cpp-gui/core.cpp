#include <unordered_map>

#include "core.hpp"

Def* Def::with_key(Key* key) {
    assert(this->key == nullptr);
    // @widget-def-ignore-keys
    assert(dynamic_cast<Widget_Def*>(this) == nullptr);

    this->key = key;
    return this;
}

Widget* Def::get_widget(Gui* gui) {
    assert(this->used == false);

    auto widget = this->on_get_widget(gui);

    // @widget-def-ignore-keys
    auto is_widget_def = dynamic_cast<Widget_Def*>(this) != nullptr;
    if(is_widget_def == false) {
        std::swap(widget->key, this->key);
    }

    this->used = true;
    return widget;
}

Def::~Def() {
    if(this->key != nullptr) {
        delete this->key;
        this->key = nullptr;
    }
}



void Widget::adopt(Widget* child) {
    assert(child->parent == nullptr);

    if(child->owner == nullptr) {
        child->owner = this;
    }

    child->parent = this;

    // NOTE(llw): Widgets created with "new" don't have their gui set.
    child->gui = gui;
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


Bool Widget::try_match(Def* def) {
    // @widget-def-ignore-keys
    auto widget_def = dynamic_cast<Widget_Def*>(def);
    if(widget_def != nullptr) {
        return this == widget_def->widget;
    }

    auto keys_match =
           (def->key == nullptr && this->key == nullptr)
        || (def->key != nullptr && def->key->equal_to(this->key));

    return keys_match && this->on_try_match(def);
}

Widget* Widget::reconcile(Widget* old_widget, Def* new_def, Bool adopt) {
    if(old_widget != nullptr && old_widget->try_match(new_def)) {
        return old_widget;
    }
    else {
        this->drop_maybe(old_widget);

        auto new_widget = new_def->get_widget(gui);
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

    auto get_old_widget_by_index_and_mark_as_used = [&](Uint old_index) {
        auto old_widget = old_widgets[old_index];
        assert(old_widget != nullptr);

        old_widgets[old_index] = nullptr;
        return old_widget;
    };


    auto new_widgets = List<Widget*> {};
    new_widgets.resize(new_defs.size());

    // Process widget and keyed defs first.
    //  Otherwise, the referenced widgets could be reconciled against defs
    //  earlier in the list:
    //   - new_defs[1] references old_children[0].
    //   - but new_defs[0] is processed first and is reconciled against
    //     old_children[0].
    for(Uint def_index = 0; def_index < new_defs.size(); def_index += 1) {
        auto new_def = new_defs[def_index];

        // @widget-def-ignore-keys
        auto widget_def = dynamic_cast<Widget_Def*>(new_def);
        if(widget_def != nullptr) {
            auto old_widget = (Widget*)nullptr;

            auto it = widget_to_index.find(widget_def->widget);
            if(it != widget_to_index.end()) {
                old_widget = get_old_widget_by_index_and_mark_as_used(it->second);
            }

            new_widgets[def_index] = this->reconcile(old_widget, new_def, adopt);
        }
        else if(new_def->key != nullptr) {
            auto old_widget = (Widget*)nullptr;

            auto it = key_to_index.find(new_def->key);
            if(it != key_to_index.end()) {
                old_widget = get_old_widget_by_index_and_mark_as_used(it->second);
            }

            new_widgets[def_index] = this->reconcile(old_widget, new_def, adopt);
        }

        // NOTE(llw): The above calls to reconcile will do some redundant work
        //  (comparing widget pointers or keys).
        //  The code is written in this way for clarity and to avoid duplicating
        //  the reconcilation logic.
        //  It would be nice to have a compiler that reliably inlines these
        //  calls and removes the redundant code.
    }


    auto old_cursor = Uint(0);

    for(Uint def_index = 0; def_index < new_defs.size(); def_index += 1) {
        auto new_def = new_defs[def_index];

        // Def already processed?
        if(new_widgets[def_index] != nullptr) {
            continue;
        }

        // Find first unused old widget.
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

