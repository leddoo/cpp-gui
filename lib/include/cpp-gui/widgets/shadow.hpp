#pragma once

#include <cpp-gui/core/widget.hpp>


struct Shadow_Fields {
    V4f     shadow_color         = { 0.0f, 0.0f, 0.0f, 0.2f };
    V2f     shadow_scale         = { 1.0f, 1.0f };
    V2f     shadow_size_delta    = { 0.0f, 0.0f };
    V2f     shadow_offset        = { 0.0f, 2.0f };
    Float32 shadow_blur_radius   = 3.0f;
    Float32 shadow_corner_radius = -1.0f; // negative to use widget's radius.

    V2f get_effective_size(V2f base_size);
};


struct Shadow_Def : virtual Def, Shadow_Fields {
    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Shadow_Widget : virtual Widget, Shadow_Fields {
    virtual void match(const Shadow_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_paint(ID2D1RenderTarget* target);
};

