#pragma once

#include <cpp-gui/widgets/single_child.hpp>


struct Padding_Def : virtual Single_Child_Def {
    V2f pad_min;
    V2f pad_max;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Padding_Widget : virtual Single_Child_Widget {
    V2f pad_min;
    V2f pad_max;

    virtual void match(const Padding_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_layout(Box_Constraints constraints) override;
};

