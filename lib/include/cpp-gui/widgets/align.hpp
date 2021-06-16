#pragma once

#include <cpp-gui/widgets/single_child.hpp>


struct Align_Def : virtual Single_Child_Def {
    V2f align_point;

    virtual Widget* on_get_widget(Gui* gui) override;
};


struct Align_Widget : virtual Single_Child_Widget {
    V2f align_point;

    virtual void match(const Align_Def& def);
    virtual Bool on_try_match(Def* def) override;

    virtual void on_layout(Box_Constraints constraints) override;
};
