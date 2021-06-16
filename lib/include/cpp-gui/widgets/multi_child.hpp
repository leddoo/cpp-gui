#pragma once

#include <cpp-gui/core/widget.hpp>


struct Multi_Child_Widget : virtual Widget {
    List<Widget*> children;

    virtual ~Multi_Child_Widget() override;
    virtual void on_paint(ID2D1RenderTarget* target) override;
    virtual Bool visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) override;
};
