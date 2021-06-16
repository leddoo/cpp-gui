#include <cpp-gui/core/widget.hpp>
#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/d2d.hpp>



static Bool hit_test_helper(Widget* widget, V2f point, std::function<Bool(Widget*)> should_stop, List<Widget*>* result) {
    // Note: widget was hit.

    // First, recurse for front-to-back order.
    auto stopped = widget->visit_children_for_hit_testing([&](Widget* child) {
        auto query = point - child->position;
        return child->on_hit_test(query)
            && hit_test_helper(child, query, should_stop, result);
    }, point);

    if(stopped) {
        return true;
    }

    // Nothing in front blocked this widget -> add to hit list.
    result->push_back(widget);
    return should_stop(widget);
}

List<Widget*> Widget::hit_test(V2f point, std::function<Bool(Widget*)> should_stop) {
    auto result = List<Widget*>();

    if(this->on_hit_test(point)) {
        hit_test_helper(this, point, should_stop, &result);
    }

    return result;
}



void Widget::mark_for_layout() {
    gui->request_frame();
}

void Widget::layout(Box_Constraints constraints) {
    this->on_layout(constraints);
}



void Widget::mark_for_paint() {
    gui->request_frame();
}

void Widget::paint(ID2D1RenderTarget* target) {
    auto old_tfx = D2D_MATRIX_3X2_F {};
    target->GetTransform(&old_tfx);
    target->SetTransform(old_tfx * D2D1::Matrix3x2F::Translation(to_d2d_sizef(this->position)));

    if(gui->draw_widget_rects) {
        auto brush = (ID2D1SolidColorBrush*)nullptr;
        auto hr = target->CreateSolidColorBrush({ 1.0f, 0.0f, 1.0f, 0.5f }, &brush);

        if(SUCCEEDED(hr)) {
            target->DrawRectangle({ 0.5f, 0.5f, this->size.x - 0.5f, this->size.y - 0.5f }, brush);
            brush->Release();
        }
    }

    this->on_paint(target);

    target->SetTransform(old_tfx);
}




V2f Widget::get_offset_from(Widget* ancestor) const {
    auto current = this;

    auto result = V2f { 0, 0 };

    while(current != nullptr) {
        if(current == ancestor) {
            return result;
        }

        result = result + current->position;
        current = current->parent;
    }

    // TODO: error.
    throw Unreachable();
}