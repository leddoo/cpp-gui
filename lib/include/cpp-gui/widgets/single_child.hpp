#pragma once

#include <cpp-gui/core/widget.hpp>


struct Single_Child_Widget : virtual Widget {
    Widget* child;

    virtual ~Single_Child_Widget() override;
    virtual void on_layout(Box_Constraints constraints) override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
    virtual Bool visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) override;
};

