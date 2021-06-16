#include <cpp-gui/core/widget.hpp>
#include <cpp-gui/core/gui.hpp>


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

