#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/base_button.hpp>
#include <cpp-gui/win32.hpp>


Widget* Base_Button_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Base_Button_Widget>(*this);
}



void Base_Button_Widget::match(const Base_Button_Def& def) {
    Single_Child_Widget::match(def);
    this->use_keyboard = def.use_keyboard;
}

Bool Base_Button_Widget::on_try_match(Def* def) {
    return try_match_t<Base_Button_Def>(this, def);
}


inline Bool started_pressing(Base_Button_Widget* button, Bool pressed_before) {
    return pressed_before == false && button->pressed() == true;
}
inline Bool stopped_pressing(Base_Button_Widget* button, Bool pressed_before) {
    return pressed_before == true && button->pressed() == false;
}

inline void maybe_send_press_begin(Base_Button_Widget* button, Bool pressed_before) {
    if(started_pressing(button, pressed_before)) {
        button->on_press_begin();
    }
}
inline void maybe_send_press_end(Base_Button_Widget* button, Bool pressed_before) {
    if(stopped_pressing(button, pressed_before)) {
        button->on_press_end();
    }
}


void Base_Button_Widget::on_key_down(Win32_Virtual_Key key) {
    auto pressed_before = this->pressed();

    if(key == VK_RETURN) {
        this->keyboard_pressing = true;
        maybe_send_press_begin(this, pressed_before);
    }
    else if(key == VK_ESCAPE) {
        this->keyboard_pressing = false;
        this->mouse_pressing    = false;
        maybe_send_press_end(this, pressed_before);
    }
}

void Base_Button_Widget::on_key_up(Win32_Virtual_Key key) {
    if(key == VK_RETURN) {
        auto pressed_before = this->pressed();
        this->keyboard_pressing = false;

        if(stopped_pressing(this, pressed_before)) {
            this->on_click_keyboard();
            this->on_click();
        }

        maybe_send_press_end(this, pressed_before);
    }
}

void Base_Button_Widget::on_lose_keyboard_focus() {
    auto pressed_before = this->pressed();
    this->keyboard_pressing = false;
    maybe_send_press_end(this, pressed_before);
}


void Base_Button_Widget::on_mouse_enter() {
    this->mouse_hovering = true;
    this->on_hover_begin();
}

void Base_Button_Widget::on_mouse_leave() {
    this->mouse_hovering = false;
    this->on_hover_end();
}


Bool Base_Button_Widget::on_mouse_down(Mouse_Button button) {
    if(button == Mouse_Button::left && this->mouse_hovering) {
        auto pressed_before = this->pressed();
        this->mouse_pressing = true;

        maybe_send_press_begin(this, pressed_before);

        this->grab_mouse_focus();
        if(this->use_keyboard) {
            this->grab_keyboard_focus();
        }
    }

    return true;
}

Bool Base_Button_Widget::on_mouse_up(Mouse_Button button) {
    if(button == Mouse_Button::left) {
        auto pressed_before = this->pressed();
        this->mouse_pressing = false;

        if(this->mouse_hovering && stopped_pressing(this, pressed_before)) {
            this->on_click_mouse();
            this->on_click();
        }

        maybe_send_press_end(this, pressed_before);

        this->release_mouse_focus();
    }

    return true;
}

void Base_Button_Widget::on_lose_mouse_focus() {
    auto pressed_before = this->pressed();
    this->mouse_pressing = false;
    maybe_send_press_end(this, pressed_before);
}

