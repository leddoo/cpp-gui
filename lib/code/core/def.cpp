#include <cpp-gui/core/widget.hpp>

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

