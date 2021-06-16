#pragma once

#include <cpp-gui/widgets/single_child.hpp>


struct Base_Button_Def : virtual Single_Child_Def {
    Bool use_keyboard = true;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Base_Button_Widget : virtual Single_Child_Widget {
    Bool use_keyboard;

    // State.
    Bool keyboard_pressing;
    Bool mouse_pressing;
    Bool mouse_hovering;

    Bool pressed() { return keyboard_pressing || mouse_pressing; }
    Bool hovered() { return mouse_hovering; }


    virtual void match(const Base_Button_Def& def);
    virtual Bool on_try_match(Def* def) override;


    // User overridable handlers.

    virtual void on_click() {}
    virtual void on_click_keyboard() {}
    virtual void on_click_mouse() {}

    virtual void on_hover_begin() {}
    virtual void on_hover_end() {}

    virtual void on_press_begin() {}
    virtual void on_press_end() {}


    // Widget handlers.

    virtual void on_key_down(Win32_Virtual_Key key) override;
    virtual void on_key_up(Win32_Virtual_Key key) override;

    virtual void on_lose_keyboard_focus() override;


    virtual Bool blocks_mouse()      override { return true; }
    virtual Bool takes_mouse_input() override { return true; }

    virtual void on_mouse_enter() override;
    virtual void on_mouse_leave() override;

    virtual Bool on_mouse_down(Mouse_Button button) override;
    virtual Bool on_mouse_up(Mouse_Button button) override;

    virtual void on_lose_mouse_focus() override;
};

