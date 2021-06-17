#pragma once

#include <cpp-gui/core/widget.hpp>


struct Rounded_Def : virtual Def {
    Float32 corner_radius;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Rounded_Widget : virtual Widget {
    Float32 corner_radius;

    static Float32 get_corner_radius(Widget* widget);
    static Float32 get_effective_corner_radius(Float32 base_radius, V2f size);
    static Float32 get_effective_corner_radius(Widget* widget);

    Float32 get_effective_corner_radius();


    virtual void match(const Rounded_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual Bool on_hit_test(V2f point) override;
};

