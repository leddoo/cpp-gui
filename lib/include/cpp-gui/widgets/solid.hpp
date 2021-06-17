#pragma once

#include <cpp-gui/core/widget.hpp>


struct Solid_Def : virtual Def {
    V4f fill_color;
    V4f stroke_color;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Solid_Widget : virtual Widget {
    V4f fill_color;
    V4f stroke_color;

    virtual void match(const Solid_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual Bool blocks_mouse() override { return true; }

    virtual void on_paint(ID2D1RenderTarget* target);
};

