#include <cpp-gui/core/widget.hpp>


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

