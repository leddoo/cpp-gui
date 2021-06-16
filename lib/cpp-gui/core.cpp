#include <unordered_map>
#include <unordered_set>

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
            delete child;
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
    return gui->set_keyboard_focus(this);
}

Bool Widget::release_keyboard_focus() {
    if(gui->get_keyboard_focus() == this) {
        gui->set_keyboard_focus(nullptr);
        return true;
    }
    else {
        return false;
    }
}


Bool Widget::grab_mouse_focus() {
    return gui->set_mouse_focus(this);
}

Bool Widget::release_mouse_focus() {
    if(gui->get_mouse_focus() == this) {
        gui->set_mouse_focus(nullptr);
        return true;
    }
    else {
        return false;
    }
}


static Bool hit_test_helper(Widget* widget, V2f point, std::function<Bool(Widget*)> should_stop, List<Widget*>* result) {
    // Note: widget was hit.

    // First, recurse for front-to-back order.
    auto stopped = widget->visit_children_for_hit_testing([&](Widget* child) {
        auto query = point - child->position;
        return child->on_hit_test(query)
            && hit_test_helper(child, query, should_stop, result);
    }, point);

    if(stopped) {
        return true;
    }

    // Nothing in front blocked this widget -> add to hit list.
    result->push_back(widget);
    return should_stop(widget);
}

List<Widget*> Widget::hit_test(V2f point, std::function<Bool(Widget*)> should_stop) {
    auto result = List<Widget*>();

    if(this->on_hit_test(point)) {
        hit_test_helper(this, point, should_stop, &result);
    }

    return result;
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


// Default implementations for user overridable handlers.

void Widget::on_create() {}

Widget::~Widget() {
    assert(this->owner == nullptr && this->parent == nullptr);

    this->release_keyboard_focus();
    this->release_mouse_focus();

    safe_delete(&this->key);
}

Bool Widget::on_try_match(Def* def) {
    UNUSED(def);
    return false;
}

void Widget::on_layout(Box_Constraints constraints) { UNUSED(constraints); }

void Widget::on_paint(ID2D1RenderTarget* target) { UNUSED(target); }

void Widget::on_gain_keyboard_focus() {}
void Widget::on_lose_keyboard_focus() {}

void Widget::on_key_down(Win32_Virtual_Key key) { UNUSED(key); }
void Widget::on_key_up(Win32_Virtual_Key key) { UNUSED(key); }
void Widget::on_char(Ascii_Char ch) { UNUSED(ch); }

Bool Widget::on_hit_test(V2f point) {
    return point >= V2f { 0, 0 } && point < this->size;
}

Bool Widget::visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) {
    UNUSED(visitor);
    UNUSED(point);
    return false;
}

Bool Widget::blocks_mouse() {
    return false;
}

Bool Widget::takes_mouse_input() {
    return false;
}


void Widget::on_gain_mouse_focus() {}
void Widget::on_lose_mouse_focus() {}

void Widget::on_mouse_enter() { }
void Widget::on_mouse_leave() { }

Bool Widget::on_mouse_down(Mouse_Button button) {
    UNUSED(button);
    return false;
}

Bool Widget::on_mouse_up(Mouse_Button button) {
    UNUSED(button);
    return false;
}

Bool Widget::on_mouse_move() {
    return false;
}




void Gui::request_frame() {
    if(!this->has_requested_frame) {
        this->request_frame_callback();
        this->has_requested_frame = true;
    }
}


Widget* Gui::get_keyboard_focus() {
    return this->keyboard_focus_widget;
}

Bool Gui::set_keyboard_focus(Widget* new_focus) {
    auto old_focus = this->keyboard_focus_widget;
    if(new_focus == old_focus) {
        return true;
    }
    else {
        if(old_focus != nullptr) {
            old_focus->on_lose_keyboard_focus();
        }

        this->keyboard_focus_widget = new_focus;

        if(new_focus != nullptr) {
            new_focus->on_gain_keyboard_focus();
        }

        return false;
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


Widget* Gui::get_mouse_focus() {
    return this->mouse.focus_widget;
}

Bool Gui::set_mouse_focus(Widget* new_focus) {
    auto old_focus = this->mouse.focus_widget;
    if(new_focus == old_focus) {
        return true;
    }
    else {
        if(old_focus != nullptr) {
            old_focus->on_lose_mouse_focus();
        }

        this->mouse.focus_widget = new_focus;

        if(new_focus != nullptr) {
            new_focus->on_gain_mouse_focus();
        }

        this->update_mouse();

        return false;
    }
}


template <typename F>
inline void send_mouse_event_to_focus_or_hot_set(Gui* gui, F send_event) {
    if(gui->mouse.focus_widget != nullptr) {
        send_event(gui->mouse.focus_widget);
    }
    else {
        for(auto widget : gui->mouse.hot_list) {
            if(send_event(widget)) {
                break;
            }
        }
    }
}


void Gui::update_mouse(Bool send_move_events) {
    auto hit_test = this->root_widget->hit_test(this->mouse.position, &Widget::blocks_mouse);

    auto new_list = List<Widget*>();
    new_list.reserve(hit_test.size());

    // Reverse hit_test to be back-to-front and filter it.
    for(auto it = hit_test.rbegin(); it != hit_test.rend(); ++it) {
        auto widget = *it;

        auto should_receive_events = 
               this->mouse.focus_widget == nullptr
            || this->mouse.focus_widget == widget;

        if(widget->takes_mouse_input() && should_receive_events) {
            new_list.push_back(widget);
        }
    }

    auto &old_list = this->mouse.hot_list; // note: reference.

    auto old_set = std::unordered_set<Widget*>(old_list.begin(), old_list.end());
    auto new_set = std::unordered_set<Widget*>(new_list.begin(), new_list.end());

    // Generate leave messages.
    for(auto widget : old_list) {
        if(contains(new_set, widget) == false) {
            widget->on_mouse_leave();
        }
    }

    // Generate enter messages.
    for(auto widget : new_list) {
        if(contains(old_set, widget) == false) {
            widget->on_mouse_enter();
        }
    }

    this->mouse.hot_list = new_list;

    // Generate move messages.
    if(send_move_events) {
        send_mouse_event_to_focus_or_hot_set(this, [](Widget* widget) {
            return widget->on_mouse_move();
        });
    }
}


void Gui::on_mouse_button(Mouse_Button button, Bool new_state, V2f position) {
    this->on_mouse_move(position);

    if(this->mouse.button_states[button] == new_state) {
        return;
    }
    this->mouse.button_states[button] = new_state;

    if(new_state == true) {
        send_mouse_event_to_focus_or_hot_set(this, [=](Widget* widget) {
            return widget->on_mouse_down(button); 
        });
    }
    else {
        send_mouse_event_to_focus_or_hot_set(this, [=](Widget* widget) {
            return widget->on_mouse_up(button); 
        });
    }
}

void Gui::on_mouse_move(V2f position) {
    if(this->mouse.entered && position == this->mouse.position) {
        return;
    }
    this->mouse.entered = true;

    auto delta = position - this->mouse.position;
    this->mouse.delta    = delta;
    this->mouse.position = position;

    this->update_mouse(true);
}

void Gui::on_mouse_leave() {
    if(this->mouse.entered == false) {
        return;
    }
    this->mouse.entered = false;

    for(auto widget : this->mouse.hot_list) {
        widget->on_mouse_leave();
    }
    this->mouse.hot_list.clear();
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



Single_Child_Widget::~Single_Child_Widget() {
    this->drop_maybe(this->child);
    this->child = nullptr;
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

