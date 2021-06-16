#pragma once

#include <cpp-gui/core/widget.hpp>


struct Single_Child_Def : virtual Def {
    Def* child;

    virtual ~Single_Child_Def();
};


struct Single_Child_Widget : virtual Widget {
    Widget* child;

    virtual ~Single_Child_Widget() override;

    virtual void match(const Single_Child_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_layout(Box_Constraints constraints) override;

    virtual void on_paint(ID2D1RenderTarget* target) override;

    virtual Bool visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) override;
};

