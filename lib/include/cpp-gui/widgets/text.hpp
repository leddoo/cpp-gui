#pragma once

#include <cpp-gui/core/widget.hpp>
#include <cpp-gui/text.hpp>


struct Text_Def : virtual Def {
    String     string;
    Font_Face* font_face;
    Float32    size;
    V4f        color;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Text_Widget : virtual Widget {
    Text_Layout layout;
    V4f         color;

    virtual ~Text_Widget();

    virtual void match(const Text_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_paint(ID2D1RenderTarget* target) override;
};

