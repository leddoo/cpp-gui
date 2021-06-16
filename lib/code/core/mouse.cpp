#include <unordered_set>

#include <cpp-gui/core/widget.hpp>
#include <cpp-gui/core/gui.hpp>


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

