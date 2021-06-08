#pragma once

#include "common.hpp"

#include <functional>
using Void_Callback = std::function<void(void)>;

struct Gui;
struct Widget;
struct Props;
struct Prop_Widget;


struct Gui_Node {
    virtual ~Gui_Node() {}
    virtual void on_destroy() {}
};


struct Widget : virtual Gui_Node {
    Gui* gui;

    Widget* owner;  // mandatory.
    Widget* parent; // optional (temporarily).

    V2f position;
    V2f size;
    V2f baseline;

    virtual void on_create() {}
    virtual void on_layout(Box_Constraints constraints) { UNUSED(constraints); }
    virtual void on_paint(ID2D1RenderTarget* target)    { UNUSED(target);      }

    void adopt(Widget* child);
    void drop(Widget* child);
    void drop_maybe(Widget* child) { if(child != nullptr) { this->drop(child); } }

    Widget* reconcile(Widget* child, Gui_Node* node);
    Widget* reconcile_widget(Widget* child, Widget* widget);
    Widget* reconcile_props(Widget* child, Props* props);

    void mark_for_layout();
    void layout(Box_Constraints constraints);

    void mark_for_paint();
    void paint(ID2D1RenderTarget* target);
};



struct Single_Child_Widget : virtual Widget {
    Widget* child;

    virtual void on_destroy() override;
    virtual void on_layout(Box_Constraints constraints) override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
};


struct Root_Widget : virtual Single_Child_Widget {};

struct Gui {
    Root_Widget root_widget;

    Void_Callback request_frame_callback;
    Bool          has_requested_frame;

    void request_frame();


    void create(Gui_Node* root, Void_Callback request_frame);
    void destroy();

    void render_frame(V2f size, ID2D1RenderTarget* target);


    Widget* inflate_widget(Widget* parent, Props* props);

    void destroy_widget(Widget* widget);
    void destroy_props(Props* props);
};



struct Props : virtual Gui_Node {
    virtual Prop_Widget* create_widget() = 0;
};

struct Prop_Widget : virtual Widget {
    virtual Bool on_props_changed(Props* raw_props) = 0;
};

