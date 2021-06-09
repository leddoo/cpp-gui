#pragma once

#include "common.hpp"

#include <functional>
using Void_Callback = std::function<void(void)>;

struct Def;
struct Widget_Def;
struct Key;
struct Widget;
struct Gui;


struct Def {
    // Without the initialization, the Widget_Def constructor does not zero this
    // field. This causes the destructor to free a nonsensical address.
    Key* key = nullptr;

    // For debugging.
    Bool used = false;

    Def* with_key(Key* key);
    Widget* get_widget(Gui* gui);

    virtual ~Def();
    virtual Widget* on_get_widget(Gui* gui) = 0;
};

struct Widget_Def : virtual Def {
    Widget* widget;

    Widget_Def(Widget* widget) : widget(widget) {}

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

    void adopt(Widget* child);
    void drop(Widget* child);
    void drop_maybe(Widget* child);

    Bool try_match(Def* def);

    Widget* reconcile(Widget* old_widget, Def* new_def, Bool adopt = true);
    List<Widget*> reconcile_list(List<Widget*> old_widgets, List<Def*> new_defs, Bool adopt = true);

    void mark_for_layout();
    void layout(Box_Constraints constraints);

    void mark_for_paint();
    void paint(ID2D1RenderTarget* target);


    // User overridable handlers:

    virtual ~Widget() {}

    virtual void on_layout(Box_Constraints constraints) {
        UNUSED(constraints);
    }

    virtual void on_paint(ID2D1RenderTarget* target) {
        UNUSED(target);
    }

    // Called after processing Widget_Defs and only if keys match.
    virtual Bool on_try_match(Def* def) {
        UNUSED(def);
        return false;
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

    Void_Callback request_frame_callback;
    Bool          has_requested_frame;

    void request_frame();


    void create(Def* root_def, Void_Callback request_frame);
    void destroy();

    void render_frame(V2f size, ID2D1RenderTarget* target);


    template <typename Some_Widget>
    Some_Widget* create_widget() {
        auto widget = new Some_Widget();
        widget->gui = this;
        return widget;
    }

    template <typename Some_Widget, typename Some_Def>
    Some_Widget* create_widget_and_match(const Some_Def& def) {
        auto widget = this->create_widget<Some_Widget>();
        widget->match(def);
        return widget;
    }

    void destroy_widget(Widget* widget);
};




struct Uint_Key : virtual Key {
    Uint value;

    Uint_Key(Uint value) : value(value) {}

    virtual Bool equal_to(const Key* other) const final override;
    virtual Uint hash() const final override;
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
};


struct Multi_Child_Widget : virtual Widget {
    List<Widget*> children;

    virtual ~Multi_Child_Widget() override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
};


struct Root_Widget : virtual Single_Child_Widget {};

