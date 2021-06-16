#pragma once

#include <cpp-gui/core/widget.hpp>


struct Multi_Child_Def : virtual Def {
    List<Def*> children;

    virtual ~Multi_Child_Def();

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Multi_Child_Widget : virtual Widget {
    List<Widget*> children;

    virtual ~Multi_Child_Widget() override;

    virtual void match(const Multi_Child_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_paint(ID2D1RenderTarget* target) override;

    virtual Bool visit_children_for_hit_testing(std::function<Bool(Widget* child)> visitor, V2f point) override;
};
