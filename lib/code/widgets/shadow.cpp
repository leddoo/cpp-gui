#include <cpp-gui/widgets/shadow.hpp>
#include <cpp-gui/widgets/rounded.hpp>
#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/d2d.hpp>

#include <d2d1_1.h>
#include <d2d1effects.h>


V2f Shadow_Fields::get_effective_size(V2f base_size) {
    return this->shadow_scale*base_size + this->shadow_size_delta;
}



Widget* Shadow_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Shadow_Widget>(*this);
}



void Shadow_Widget::match(const Shadow_Def& def) {
    *(Shadow_Fields*)this = *(Shadow_Fields*)&def;
    this->mark_for_paint();
}

Bool Shadow_Widget::on_try_match(Def* def) {
    return try_match_t<Shadow_Def>(this, def);
}


void Shadow_Widget::on_paint(ID2D1RenderTarget* target) {
    if(this->shadow_color.a == 0.0f) {
        return;
    }

    auto effective_size = this->get_effective_size(this->size);

    auto corner_radius = this->shadow_corner_radius;
    if(corner_radius < 0.0f) {
        corner_radius = Rounded_Widget::get_effective_corner_radius(this);
    }
    else {
        corner_radius = Rounded_Widget::get_effective_corner_radius(corner_radius, effective_size);
    }

    auto hr = HRESULT();

    // create off-screen buffer.
    auto buffer = (ID2D1BitmapRenderTarget*)nullptr;
    hr = target->CreateCompatibleRenderTarget(
        to_d2d_sizef(effective_size), &buffer
    );
    if(!SUCCEEDED(hr)) { return; }
    defer { buffer->Release(); };

    // draw rounded rectangle into buffer.
    buffer->BeginDraw();
    {
        buffer->Clear({ 0, 0, 0, 0 });

        auto brush = (ID2D1SolidColorBrush*)nullptr;
        hr = buffer->CreateSolidColorBrush(to_d2d_colorf(this->shadow_color), &brush);
        if(!SUCCEEDED(hr)) { return; }
        defer { brush->Release(); };

        auto rect = D2D1::RectF(0, 0, effective_size.x, effective_size.y);
        buffer->FillRoundedRectangle(
            D2D1::RoundedRect(rect, corner_radius, corner_radius),
            brush
        );
    }
    buffer->EndDraw();

    // get buffer's bitmap.
    auto bitmap = (ID2D1Bitmap*)nullptr;
    hr = buffer->GetBitmap(&bitmap);
    if(!SUCCEEDED(hr)) { return; }
    defer { bitmap->Release(); };

    // get device context.
    auto context = (ID2D1DeviceContext*)nullptr;
    hr = target->QueryInterface(&context);
    if(!SUCCEEDED(hr)) { return; }
    defer { context->Release(); };

    // create blur.
    auto blur = (ID2D1Effect*)nullptr;
    hr = context->CreateEffect(CLSID_D2D1GaussianBlur, &blur);
    if(!SUCCEEDED(hr)) { return; }
    defer { blur->Release(); };

    // configure blur.
    blur->SetInput(0, bitmap);
    hr = blur->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, this->shadow_blur_radius/3.0f);
    if(!SUCCEEDED(hr)) { return; }

    // draw blur.
    auto effective_offset = this->shadow_offset - 0.5f*(effective_size - this->size);
    context->DrawImage(blur, to_d2d_point2f(effective_offset));
}

