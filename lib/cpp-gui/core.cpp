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



void Widget::become_parent(Widget* child) {
    // detect widgets created with new.
    assert(child->gui != nullptr);

    assert(child->parent == nullptr);

    if(child->owner == nullptr) {
        child->owner = this;
    }

    child->parent = this;
}

void Widget::become_owner(Widget* child) {
    // detect widgets created with new.
    assert(child->gui != nullptr);

    assert(child->owner == nullptr);
    child->owner = this;
}

void Widget::transfer_ownership(Widget* child, Widget* new_owner) {
    assert(child->owner == this);
    child->owner = new_owner;
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

Widget* Widget::reconcile(
        Widget* old_widget, Def* new_def,
        New_Child_Action new_child_action
) {
    if(old_widget != nullptr && old_widget->try_match(new_def)) {
        return old_widget;
    }
    else {
        this->drop_maybe(old_widget);

        auto new_widget = new_def->get_widget(gui);

        if(new_child_action == New_Child_Action::become_parent) {
            this->become_parent(new_widget);
        }
        else if(new_child_action == New_Child_Action::become_owner) {
            this->become_owner(new_widget);
        }
        else {
            assert(new_child_action == New_Child_Action::none);
        }

        return new_widget;
    }
}

List<Widget*> Widget::reconcile_list(
    List<Widget*>& old_widgets, const List<Def*>& new_defs,
    New_Child_Action new_child_action
) {
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

            new_widgets[def_index] = this->reconcile(old_widget, new_def, new_child_action);
        }
        else if(new_def->key != nullptr) {
            auto old_widget = (Widget*)nullptr;

            auto it = key_to_index.find(new_def->key);
            if(it != key_to_index.end()) {
                old_widget = get_old_widget_by_index_and_mark_as_used(it->second);
            }

            new_widgets[def_index] = this->reconcile(old_widget, new_def, new_child_action);
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

        new_widgets[def_index] = this->reconcile(old_widget, new_def, new_child_action);
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

    if(gui->draw_widget_rects) {
        auto brush = (ID2D1SolidColorBrush*)nullptr;
        auto hr = target->CreateSolidColorBrush({ 1.0f, 0.0f, 1.0f, 0.5f }, &brush);

        if(SUCCEEDED(hr)) {
            target->DrawRectangle({ 0.5f, 0.5f, this->size.x - 0.5f, this->size.y - 0.5f }, brush);
            brush->Release();
        }
    }

    this->on_paint(target);

    target->SetTransform(old_tfx);
}


Bool Widget::grab_keyboard_focus() {
    if(gui->keyboard_focus_widget == this) {
        return true;
    }
    else {
        if(gui->keyboard_focus_widget != nullptr) {
            gui->keyboard_focus_widget->on_lose_keyboard_focus();
        }

        gui->keyboard_focus_widget = this;
        this->on_gain_keyboard_focus();

        return false;
    }
}

Bool Widget::release_keyboard_focus() {
    if(gui->keyboard_focus_widget == this) {
        this->on_lose_keyboard_focus();
        gui->keyboard_focus_widget = nullptr;
        return true;
    }
    else {
        return false;
    }
}


V2f Widget::get_offset_from(Widget* ancestor) const {
    auto current = this;

    auto result = V2f { 0, 0 };

    while(current != nullptr) {
        if(current == ancestor) {
            return result;
        }

        result = result + current->position;
        current = current->parent;
    }

    // TODO: error.
    throw Unreachable();
}



void Gui::request_frame() {
    if(!this->has_requested_frame) {
        this->request_frame_callback();
        this->has_requested_frame = true;
    }
}


void Gui::on_key_down(Win32_Virtual_Key key) {
    if(this->keyboard_focus_widget != nullptr) {
        this->keyboard_focus_widget->on_key_down(key);
    }
}

void Gui::on_key_up(Win32_Virtual_Key key) {
    if(this->keyboard_focus_widget != nullptr) {
        this->keyboard_focus_widget->on_key_up(key);
    }
}

void Gui::on_char(Uint16 ch) {
    auto is_ascii_printable = ch >= 0x20 && ch <= 0x7E;
    if(is_ascii_printable && this->keyboard_focus_widget != nullptr) {
        this->keyboard_focus_widget->on_char((Ascii_Char)ch);
    }
}


void Gui::create(Def* root_def, Void_Callback request_frame) {
    this->request_frame_callback = request_frame;

    this->root_widget         = new Root_Widget();
    this->root_widget->gui    = this;
    this->root_widget->owner  = this->root_widget;
    this->root_widget->parent = this->root_widget;
    this->root_widget->child  = nullptr;
    if(root_def != nullptr) {
        this->set_root(root_def);
    }
}

void Gui::destroy() {
    delete this->root_widget;
}

void Gui::set_root(Def* def) {
    auto root = this->root_widget;
    root->child = root->reconcile(root->child, def);
    delete def;
}


void Gui::render_frame(V2f size, ID2D1RenderTarget* target) {
    this->root_widget->layout(Box_Constraints::tight(size));
    this->root_widget->paint(target);
    this->has_requested_frame = false;
}


void Gui::destroy_widget(Widget* widget) {
    assert(widget->owner == nullptr && widget->parent == nullptr);

    if(this->keyboard_focus_widget == widget) {
        this->keyboard_focus_widget = nullptr;
    }

    if(widget->key != nullptr) {
        delete widget->key;
    }
    delete widget;
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

