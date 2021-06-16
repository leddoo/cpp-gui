#pragma once

#include <cpp-gui/common.hpp>


struct Def;
struct Key;
struct Widget;
struct Gui;
struct ID2D1RenderTarget;


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
    virtual Bool equal_to(const Key* other) = 0;
    virtual Uint hash() = 0;
};

template <typename T>
struct T_Key : virtual Key {
    T value;

    T_Key(const T& value) : value(value) {}

    virtual Bool equal_to(const Key* other) final override {
        auto _other = dynamic_cast<const T_Key<T>*>(other);
        if(_other == nullptr) {
            return false;
        }

        return this->value == _other->value;
    }

    virtual Uint hash() final override {
        return std::hash<T>()(this->value);
    }
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


    // Analogous to grab/release_keyboard_focus.
    Bool grab_mouse_focus();
    Bool release_mouse_focus();


    // Hit test.
    //  - Returns a list of hit widgets in front-to-back order (potential
    //    entries are this widget and any descendants).
    //  - `should_stop` can be used to implement blocking.
    List<Widget*> hit_test(V2f point, std::function<Bool(Widget*)> should_stop);


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

    // Visit children for hit testing.
    //  - Pass children to the visitor in front-to-back order. (Calling
    //    on_hit_test on the children is the visitor's responsibility.)
    //  - The `point` parameter can be used if this widget has some sort of
    //    spatial query optimization.
    //  - The visitor returns whether to stop passing children to it.
    //  - Return true once the visitor returns true. Otherwise, or if the widget
    //    has no children, return false.
    //  - The system generally only calls this procedure if on_hit_test returned
    //    true for this widget.
    virtual Bool visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point);

    // Property: Blocks mouse.
    //  - Returns whether this widget should currently block the mouse from
    //    interacting with widgets below this widget.
    //  - Default: False.
    //  - TBD: something you can call when this property changes.
    virtual Bool blocks_mouse();

    // Property: Takes mouse input.
    //  - Returns whether this widget should currently receive mouse events.
    //  - Independent of blocks_mouse.
    //  - Default: False.
    //  - TBD: something you can call when this property changes.
    virtual Bool takes_mouse_input();


    /* Receiving mouse events:
        - Hot list:
            - Widgets that take mouse input and are currently hit by the mouse
              (hit_test with should_stop = blocks_mouse).
            - In back-to-front order.
            - If there is a focus widget, the list is filtered to include at
              most the focus widget. (generates enter/leave events)
        - enter/leave events:
            - Always sent when a widget enters/leaves the hot list.
            - Leave events are always sent before enter events.
        - down/up/move events:
            - Always sent to the focus widget if there is one, even if it is not
              in the hot list.
            - Otherwise, sent to the widgets in the hot list in order.  If a
              widget's handler returns true, the widgets after it in the hot
              list won't receive the event.
    */

    virtual void on_mouse_enter();
    virtual void on_mouse_leave();

    virtual void on_gain_mouse_focus();
    virtual void on_lose_mouse_focus();

    virtual Bool on_mouse_down(Mouse_Button button);
    virtual Bool on_mouse_up(Mouse_Button button);
    virtual Bool on_mouse_move();


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



// Helper for implementing "try_match" if widget has "match".
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

