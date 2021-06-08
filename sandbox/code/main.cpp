#include <cpp-gui/core.hpp>
#include <cpp-gui/text.hpp>

#pragma comment (lib, "User32.lib")
#pragma comment (lib, "D2d1.lib")
#pragma comment (lib, "Dwrite.lib")


struct Text_Props : virtual Props {
    String     string;
    Font_Face* font_face;
    Float32    size;
    V4f        color;

    virtual Prop_Widget* create_widget() final;
};

struct Text_Widget : virtual Prop_Widget {
    Text_Layout layout;
    V4f         color;

    virtual void on_destroy() final;

    virtual void on_paint(ID2D1RenderTarget* target) final;

    virtual Bool on_props_changed(Props* raw_props) final;
};

Prop_Widget* Text_Props::create_widget() {
    return new Text_Widget {};
}


void Text_Widget::on_destroy() {
    this->layout.destroy();
    *this = {};
}

Bool Text_Widget::on_props_changed(Props* raw_props) {
    auto props = dynamic_cast<Text_Props*>(raw_props);
    if(props == nullptr) {
        return false;
    }

    this->layout.destroy();
    this->layout.create(props->font_face, props->size, props->string);

    this->color      = props->color;
    this->size       = this->layout.size;
    this->baseline.y = round(props->font_face->ascent(props->size));

    this->mark_for_paint();

    gui->destroy_props(raw_props);

    return true;
}

void Text_Widget::on_paint(ID2D1RenderTarget* target) {
    this->layout.paint(target, this->color);
}



struct Align_Props : virtual Props {
    Gui_Node* child;
    V2f       align_point;

    virtual Prop_Widget* create_widget() final;
};

struct Align_Widget : virtual Prop_Widget, virtual Single_Child_Widget {
    V2f align_point;

    virtual void on_layout(Box_Constraints constraints) final;

    virtual Bool on_props_changed(Props* raw_props) final;
};


Prop_Widget* Align_Props::create_widget() {
    return new Align_Widget {};
}


void Align_Widget::on_layout(Box_Constraints constraints) {
    // TODO: once we have sizing biases.
    //Single_Child_Widget::on_layout(constraints);
    this->child->layout(constraints);
    this->size = constraints.max;
    this->child->position = this->align_point * (this->size - this->child->size);
}

Bool Align_Widget::on_props_changed(Props* raw_props) {
    auto props = dynamic_cast<Align_Props*>(raw_props);
    if(props == nullptr) {
        return false;
    }

    this->child       = this->reconcile(this->child, props->child);
    this->align_point = props->align_point;

    this->mark_for_layout();

    gui->destroy_props(raw_props);

    return true;
}


ID2D1Factory*           d2d_factory;
ID2D1HwndRenderTarget*  d2d_render_target;

IDWriteFactory* dwrite_factory;

Font_Face normal_font_face;

Gui gui;


void draw(V2f window_size) {
    d2d_render_target->BeginDraw();
    defer {
        auto hr = d2d_render_target->EndDraw();
        assert(SUCCEEDED(hr));
    };

    d2d_render_target->Clear(D2D1::ColorF(D2D1::ColorF::White));

    gui.render_frame(window_size, d2d_render_target);
}


LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    if(message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    else if(message == WM_SIZE) {
        auto rect = RECT {};
        GetClientRect(window, &rect) ;

        auto hr = HRESULT {};

        safe_release(&d2d_render_target);

        hr = d2d_factory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(
                window,
                D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top)
            ),
            &d2d_render_target
        );
        assert(SUCCEEDED(hr));

        return 0;
    }
    else if(message == WM_PAINT) {
        auto rect = RECT {};
        GetClientRect(window, &rect);

        auto window_size = V2f { rect.right - rect.left, rect.bottom - rect.top };
        draw(window_size);

        ValidateRect(window, NULL);
        return 0;
    }
    else if(message == WM_ERASEBKGND) {
        return 1;
    }
    else {
        return DefWindowProcA(window, message, w_param, l_param);
    }
}


int main() {
    auto instance = GetModuleHandle(nullptr);

    auto window_class = WNDCLASSA {};
    window_class.style         = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc   = main_window_proc;
    window_class.hCursor       = LoadCursorA(0, IDC_ARROW);
    window_class.hIcon         = LoadIconA(0, IDI_APPLICATION);
    window_class.hInstance     = instance;
    window_class.lpszClassName = "quinn_editor_main_window_class";

    auto window_class_atom = RegisterClassA(&window_class);
    assert(window_class_atom != 0);


    auto window = CreateWindowExA(
        0,
        MAKEINTATOM(window_class_atom),
        "Quinn Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, instance, nullptr
    );
    assert(window != 0);


    // setup direct write and global fonts.
    {
        auto hr = HRESULT {};

        hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory);
        assert(SUCCEEDED(hr));

        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown **>(&dwrite_factory)
        );
        assert(SUCCEEDED(hr));



        auto system_fonts = (IDWriteFontCollection*)nullptr;
        hr = dwrite_factory->GetSystemFontCollection(&system_fonts);
        assert(SUCCEEDED(hr));

        auto family_index = (UINT32)0;
        auto family_exists = (BOOL)0;
        hr = system_fonts->FindFamilyName(
            L"Source Code Pro",
            &family_index,
            &family_exists
        );
        assert(SUCCEEDED(hr));
        assert(family_exists);

        auto family = (IDWriteFontFamily*)nullptr;
        hr = system_fonts->GetFontFamily(family_index, &family);
        assert(SUCCEEDED(hr));

        // temp normal font.
        {
            auto normal_font = (IDWriteFont*)nullptr;
            hr = family->GetFirstMatchingFont(
                DWRITE_FONT_WEIGHT_REGULAR,
                DWRITE_FONT_STRETCH_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                &normal_font
            );
            assert(SUCCEEDED(hr));

            auto normal_dwrite_font_face = (IDWriteFontFace*)nullptr;
            hr = normal_font->CreateFontFace(&normal_dwrite_font_face);
            assert(SUCCEEDED(hr));

            normal_font_face.create(normal_dwrite_font_face);
        }
    }


    auto text = new Text_Props {};
    text->string = "hello, there!";
    text->font_face = &normal_font_face;
    text->size = 48.0f;
    text->color = V4f { 0.4f, 0.6f, 0.7f, 1.0f };

    auto align = new Align_Props {};
    align->child = text;
    align->align_point = { 0.5f, 0.5f };

    auto root = align;
    auto request_frame = [=]() { InvalidateRect(window, nullptr, false); };
    gui.create(root, request_frame);


    //ShowWindow(window, SW_SHOWMAXIMIZED);
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);


    auto msg = MSG {};
    while(GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}


