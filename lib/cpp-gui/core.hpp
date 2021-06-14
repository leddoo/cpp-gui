#pragma once

#include "common.hpp"

#include <functional>
using Void_Callback = std::function<void(void)>;

using Win32_Virtual_Key = Uint8;
using Ascii_Char = Uint8;


enum class Mouse_Button : Uint8 {
    left,
    middle,
    right,

    _count
};

struct Mouse_Info {
    V2f local_position;
    V2f global_position;
    V2f delta;
    Bool button_states[(Uint)Mouse_Button::_count];
};


struct Def;
struct Widget_Def;
struct Key;
struct Widget;
struct Gui;


struct Def {
    Key* key = nullptr;

    Def* with_key(Key* key);
    Widget* get_widget(Gui* gui);

    virtual ~Def();
    virtual Widget* on_get_widget(Gui* gui) = 0;

    // For debugging.
    Bool used = false;
};

struct Widget_Def : virtual Def {
    Widget* widget;

    Widget_Def(Widget* widget) : Def(), widget(widget) {}

    virtual Widget* on_get_widget(Gui* gui) final override {
        UNUSED(gui);
        return this->widget;
    }

    // Widget_Defs can't have keys, as that would be redundant. @widget-def-ignore-keys
};



struct Key {
    virtual ~Key() {}
    virtual Bool equal_to(const Key* other) const = 0;
    virtual Uint hash() const = 0;
};



struct Widget {
    Gui* gui;

    Widget* owner;
    Widget* parent;

    Key* key;

    V2f position;
    V2f size;
    V2f baseline;

    void become_parent(Widget* child);
    void become_owner(Widget* child);
    void transfer_ownership(Widget* child, Widget* new_owner);
    void drop(Widget* child);
    void drop_maybe(Widget* child);

    Bool try_match(Def* def);


    enum class New_Child_Action {
        become_parent,
        become_owner,
        none,
    };

    Widget* reconcile(
        Widget* old_widget, Def* new_def,
        New_Child_Action new_child_action = New_Child_Action::become_parent
    );

    // NOTE: The list of old widgets is modified by the call. It should not be
    //  used after the call.
    List<Widget*> reconcile_list(
        List<Widget*>& old_widgets, const List<Def*>& new_defs,
        New_Child_Action new_child_action = New_Child_Action::become_parent
    );


    void mark_for_layout();
    void layout(Box_Constraints constraints);

    void mark_for_paint();
    void paint(ID2D1RenderTarget* target);


    // These return whether this widget had the keyboard focus before the call.
    // Widget had keyboard focus?
    // Yes: grab is a no-op.    No: release is a no-op.
    Bool grab_keyboard_focus();
    Bool release_keyboard_focus();


    V2f get_offset_from(Widget* ancestor) const;


    // User overridable handlers:

    // - Called before the widget has an owner or a parent.
    virtual void on_create();

    virtual ~Widget();

    // Try matching a definition.
    //  - If this widget supports the def, change state to match it.
    //  - Return whether the def was matched.
    //  - Default: No defs are matched.
    //  - Never called with Widget_Defs.
    //  - Only called if the def and widget keys match.
    //  - Used for reconciliation. Returning false generally implies that this
    //    widget will be destroyed and replaced by `def->get_widget()`.
    virtual Bool on_try_match(Def* def);

    // Lay out this widget and its children.
    //  - The layout includes this widget's size and baseline fields and any
    //    widget specific data (eg: child positions).
    //  - Call Widget::layout to lay out children. Not this procedure!
    virtual void on_layout(Box_Constraints constraints);

    // Paint this widget and its children.
    //  - Call Widget::paint to paint children. Not this procedure!
    virtual void on_paint(ID2D1RenderTarget* target);


    virtual void on_gain_keyboard_focus();
    virtual void on_lose_keyboard_focus();

    virtual void on_key_down(Win32_Virtual_Key key);
    virtual void on_key_up(Win32_Virtual_Key key);
    virtual void on_char(Ascii_Char ch);



    // Hit test only this widget.
    //  - Default: Returns whether the point is inside this widget's rect.
    virtual Bool on_hit_test(V2f point);

    // Hit test only the children of this widget.
    //  - Children should be processed front to back.
    //  - The callback returns whether to continue passing children to it.
    //  - The system generally only calls this procedure if on_hit_test returned
    //    true for this widget.
    virtual void on_hit_test_children(std::function<Bool(Widget* child)> callback);

    // Property: Blocks mouse.
    //  - Returns whether this widget should currently block the mouse from
    //    interacting with widgets below this widget.
    //  - Default: True.
    //  - TBD: something you can call when this property changes.
    virtual Bool blocks_mouse();

    // Property: Takes mouse input.
    //  - Returns whether this widget should currently receive mouse events.
    //  - Independent of blocks_mouse.
    //  - Default: False.
    //  - TBD: something you can call when this property changes.
    virtual Bool takes_mouse_input();

    virtual void on_mouse_enter(Mouse_Info info);
    virtual void on_mouse_leave(Mouse_Info info);

    virtual void on_mouse_down(Mouse_Button button, Mouse_Info info);
    virtual void on_mouse_up(Mouse_Button button, Mouse_Info info);
    virtual void on_mouse_move(Mouse_Info info);


    // helpers.

    template <typename B_Widget>
    B_Widget* reconcile_t(
        Widget* old_widget, Def* new_def,
        New_Child_Action new_child_action = New_Child_Action::become_parent
    ) {
        auto widget = this->reconcile(old_widget, new_def, new_child_action);
        return safe_dynamic_cast<B_Widget*>(widget);
    }

    template <typename B_Widget, typename A_Widget>
    List<B_Widget*> reconcile_list_t(
        const List<A_Widget*>& old_a_widgets, const List<Def*>& new_defs,
        New_Child_Action new_child_action = New_Child_Action::become_parent
    ) {
        auto old_widgets = map_list_cast<Widget*>(old_a_widgets);
        auto new_widgets = this->reconcile_list(old_widgets, new_defs, new_child_action);
        return map_list_safe_dynamic_cast<B_Widget*>(new_widgets);
    }

};


template <typename Some_Def, typename Some_Widget>
Bool try_match_t(Some_Widget* widget, Def* base_def) {
    auto def = dynamic_cast<Some_Def*>(base_def);
    if(def != nullptr) {
        widget->match(*def);
        return true;
    }
    else {
        return false;
    }
}



struct Root_Widget;

struct Gui {
    Root_Widget* root_widget;

    void create(Def* root_def, Void_Callback request_frame);
    void destroy();

    void set_root(Def* def);

    void render_frame(V2f size, ID2D1RenderTarget* target);


    Void_Callback request_frame_callback;
    Bool          has_requested_frame;

    void request_frame();


    // TODO: WM_SETFOCUS and WM_KILLFOCUS?
    Widget* keyboard_focus_widget;
    Uint8   keyboard_state[256];

    Bool is_key_down(Win32_Virtual_Key key) {
        return (this->keyboard_state[key] & 0x80) != 0;
    }

    Bool is_key_toggled(Win32_Virtual_Key key) {
        return (this->keyboard_state[key] & 0x01) != 0;
    }

    void on_key_down(Win32_Virtual_Key key);
    void on_key_up(Win32_Virtual_Key key);
    void on_char(Uint16 ch);


    template <typename Some_Widget>
    Some_Widget* create_widget() {
        auto widget = new Some_Widget();
        widget->gui = this;
        widget->on_create();
        return widget;
    }

    template <typename Some_Widget, typename Some_Def>
    Some_Widget* create_widget_and_match(const Some_Def& def) {
        auto widget = this->create_widget<Some_Widget>();
        widget->match(def);
        return widget;
    }

    void destroy_widget(Widget* widget);


    // For debugging.
    Bool draw_widget_rects = false;
};




template <typename T>
struct Key_T : virtual Key {
    T value;

    Key_T(const T& value) : value(value) {}

    virtual Bool equal_to(const Key* other) const final override {
        auto _other = dynamic_cast<const Key_T<T>*>(other);
        if(_other == nullptr) {
            return false;
        }

        return this->value == _other->value;
    }

    virtual Uint hash() const final override {
        return std::hash<T>()(this->value);
    }
};


// Helper for using std::unordered_map with keys.

struct Key_Pointer {
    Key* key;

    Key_Pointer(Key* key) : key(key) {}
};

inline Bool operator==(Key_Pointer left, Key_Pointer right) {
    return left.key->equal_to(right.key);
}

namespace std {
    template <>
    struct hash<Key_Pointer> {
        std::size_t operator()(Key_Pointer key_pointer) const noexcept {
            return key_pointer.key->hash();
        }
    };
}




struct Single_Child_Widget : virtual Widget {
    Widget* child;

    virtual ~Single_Child_Widget() override;
    virtual void on_layout(Box_Constraints constraints) override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
    virtual void on_hit_test_children(std::function<Bool(Widget* child)> callback) override;
};


struct Multi_Child_Widget : virtual Widget {
    List<Widget*> children;

    virtual ~Multi_Child_Widget() override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
    virtual void on_hit_test_children(std::function<Bool(Widget* child)> callback) override;
};


struct Root_Widget : virtual Single_Child_Widget {};

