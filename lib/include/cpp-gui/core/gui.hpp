#pragma once

#include <cpp-gui/common.hpp>
#include <cpp-gui/core/widget.hpp>


struct Gui {
    Widget* root_widget;

    void create(Def* root_def, Void_Callback request_frame);
    void destroy();

    void set_root(Def* def);

    void render_frame(V2f size, ID2D1RenderTarget* target);


    Void_Callback request_frame_callback;
    Bool          has_requested_frame;

    void request_frame();



    // Keyboard stuff.

    // TODO: WM_SETFOCUS and WM_KILLFOCUS?
    Widget* keyboard_focus_widget;
    Uint8   keyboard_state[256];

    Widget* get_keyboard_focus();
    Bool    set_keyboard_focus(Widget* new_focus);

    Bool is_key_down(Win32_Virtual_Key key) {
        return (this->keyboard_state[key] & 0x80) != 0;
    }

    Bool is_key_toggled(Win32_Virtual_Key key) {
        return (this->keyboard_state[key] & 0x01) != 0;
    }

    void on_key_down(Win32_Virtual_Key key);
    void on_key_up(Win32_Virtual_Key key);
    void on_char(Uint16 ch);



    // Mouse stuff.

    // TODO: clear mouse focus when lose window focus.
    struct {
        List<Widget*> hot_list;
        Widget*       focus_widget;

        V2f  position;
        V2f  delta;
        Bool button_states[Mouse_Button::_count];
        Bool entered;
    } mouse;

    void update_mouse(Bool send_move_events = false);

    Widget* get_mouse_focus();
    Bool    set_mouse_focus(Widget* new_focus);

    void on_mouse_button(Mouse_Button button, Bool new_state, V2f position);
    void on_mouse_move(V2f position);
    void on_mouse_leave();



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


    // For debugging.
    Bool draw_widget_rects = false;
};


