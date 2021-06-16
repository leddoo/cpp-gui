#include <cpp-gui/core/widget.hpp>
#include <cpp-gui/core/gui.hpp>
#include <cpp-gui/widgets/multi_child.hpp>
#include <cpp-gui/widgets/align.hpp>
#include <cpp-gui/widgets/padding.hpp>
#include <cpp-gui/widgets/text.hpp>
#include <cpp-gui/widgets/base_button.hpp>
#include <cpp-gui/text.hpp>

#pragma comment (lib, "User32.lib")
#pragma comment (lib, "D2d1.lib")
#pragma comment (lib, "Dwrite.lib")



struct Stack_Def : virtual Multi_Child_Def {
    // tbd: axis, direction, alignment.

    virtual Widget* on_get_widget(Gui* gui) final override;
};

struct Stack_Widget : virtual Multi_Child_Widget {
    // tbd: axis, direction, alignment.

    virtual void on_layout(Box_Constraints constraints) final override;

    virtual void match(const Stack_Def& def);
    virtual Bool on_try_match(Def* def) final override;
};


Widget* Stack_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Stack_Widget>(*this);
}


void Stack_Widget::on_layout(Box_Constraints constraints) {
    auto cursor = V2f {};
    auto max_height = 0.0f;
    for(auto child : this->children) {
        child->layout(constraints);
        child->position = cursor;

        cursor.x += child->size.x;
        max_height = max(max_height, child->size.y);
    }
    this->size = { cursor.x, max_height };
}

void Stack_Widget::match(const Stack_Def& def) {
    this->children = this->reconcile_list(this->children, def.children);

    this->mark_for_layout();
}

Bool Stack_Widget::on_try_match(Def* def) {
    return try_match_t<Stack_Def>(this, def);
}



struct Rect_Widget : public Widget {
    V4f color;

    virtual void on_paint(ID2D1RenderTarget* target) final override {
        auto brush = (ID2D1SolidColorBrush*)nullptr;
        auto hr = target->CreateSolidColorBrush(to_d2d_colorf(this->color), &brush);
        if(SUCCEEDED(hr)) {
            target->FillRectangle(
                D2D1::RectF(0, 0, this->size.x, this->size.y),
                brush
            );
            brush->Release();
        }
    }
};



struct Simple_Line_Edit : public Widget {
    Text_Widget* text_widget;
    Font_Face* font_face;
    String buffer;

    void update_text_widget();


    virtual void on_create() final override;
    virtual ~Simple_Line_Edit();

    virtual void on_paint(ID2D1RenderTarget* target) final override;

    virtual void on_key_down(Win32_Virtual_Key key) final override;
    virtual void on_char(Ascii_Char ch) final override;
};

void Simple_Line_Edit::update_text_widget() {
    auto def = Text_Def {};
    def.string    = this->buffer;
    def.font_face = this->font_face;
    def.size      = 24;
    def.color     = { 0, 0, 0, 1 };

    this->text_widget->match(def);
    this->size     = this->text_widget->size;
    this->baseline = this->text_widget->baseline;
    // mark layout as changed.
}

void Simple_Line_Edit::on_create() {
    this->text_widget = gui->create_widget<Text_Widget>();
    this->grab_keyboard_focus();
}

Simple_Line_Edit::~Simple_Line_Edit() {
    safe_delete(&this->text_widget);
}

void Simple_Line_Edit::on_paint(ID2D1RenderTarget* target) {
    this->text_widget->paint(target);
}

void Simple_Line_Edit::on_key_down(Win32_Virtual_Key key) {
    if(key == 'A' && gui->is_key_down(VK_CONTROL)) {
        this->buffer = gui->is_key_toggled(VK_CAPITAL) ? "ALL" : "all";
        this->update_text_widget();
    }
    else if(key == VK_BACK && this->buffer.size() > 0) {
        this->buffer.pop_back();
        this->update_text_widget();
    }
    else if(key == VK_ESCAPE) {
        this->buffer.clear();
        this->update_text_widget();
    }
}

void Simple_Line_Edit::on_char(Ascii_Char ch) {
    this->buffer.push_back(ch);
    this->update_text_widget();
}



struct Simple_Button_Def : virtual Base_Button_Def {
    V4f  color;

    virtual Widget* on_get_widget(Gui* gui) final override;
};

struct Simple_Button_Widget : virtual Base_Button_Widget {
    V4f color;

    int id;
    virtual void on_create() final override {
        static int next_id = 0;
        next_id += 1;
        this->id = next_id;
    }

    virtual void match(const Simple_Button_Def& def);
    virtual Bool on_try_match(Def* def) final override;

    virtual void on_click() override;
    virtual void on_click_keyboard() override;
    virtual void on_click_mouse() override;

    virtual void on_hover_begin() override;
    virtual void on_hover_end() override;
    virtual void on_press_begin() override;
    virtual void on_press_end() override;

    virtual void on_paint(ID2D1RenderTarget* target) final override;
};


Widget* Simple_Button_Def::on_get_widget(Gui* gui) {
    return gui->create_widget_and_match<Simple_Button_Widget>(*this);
}


void Simple_Button_Widget::match(const Simple_Button_Def& def) {
    Base_Button_Widget::match(def);
    this->color = def.color;
}

Bool Simple_Button_Widget::on_try_match(Def* def) {
    return try_match_t<Simple_Button_Def>(this, def);
}


void Simple_Button_Widget::on_click() {
    printf("%d CLICK\n", this->id);
    this->mark_for_paint();
}

void Simple_Button_Widget::on_click_keyboard() {
    printf("%d CLICK (keyboard)\n", this->id);
}

void Simple_Button_Widget::on_click_mouse() {
    printf("%d CLICK (mouse)\n", this->id);
}

void Simple_Button_Widget::on_hover_begin() {
    printf("%d hover begin\n", this->id);
    this->mark_for_paint();
}

void Simple_Button_Widget::on_hover_end() {
    printf("%d hover end\n", this->id);
    this->mark_for_paint();
}

void Simple_Button_Widget::on_press_begin() {
    printf("%d press begin\n", this->id);
    this->mark_for_paint();
}

void Simple_Button_Widget::on_press_end() {
    printf("%d press end\n", this->id);
    this->mark_for_paint();
}



void Simple_Button_Widget::on_paint(ID2D1RenderTarget* target) {
    auto color = this->color;
    if(this->hovered()) { color *= 1.1f; }
    if(this->pressed()) { color *= 1.1f; }

    auto brush = (ID2D1SolidColorBrush*)nullptr;
    auto hr = target->CreateSolidColorBrush(to_d2d_colorf(color), &brush);
    if(SUCCEEDED(hr)) {
        target->FillRectangle(
            D2D1::RectF(0, 0, this->size.x, this->size.y),
            brush
        );
        brush->Release();
    }

    Single_Child_Widget::on_paint(target);
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

    else if(message == WM_KEYDOWN) {
        GetKeyboardState(&gui.keyboard_state[0]);
        gui.on_key_down((Win32_Virtual_Key)w_param);
        return 0;
    }
    else if(message == WM_KEYUP) {
        GetKeyboardState(&gui.keyboard_state[0]);
        gui.on_key_up((Win32_Virtual_Key)w_param);
        return 0;
    }
    else if(message == WM_CHAR) {
        GetKeyboardState(&gui.keyboard_state[0]);
        gui.on_char((Uint16)w_param);
        return 0;
    }

    else if(message == WM_MOUSEMOVE) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_move(position);

        // NOTE(llw): Track mouse to get the WM_MOUSELEAVE message.
        auto tme = TRACKMOUSEEVENT();
        tme.cbSize = sizeof(TRACKMOUSEEVENT);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = window;
        tme.dwHoverTime = HOVER_DEFAULT;
        TrackMouseEvent(&tme);

        return 0;
    }
    else if(message == WM_MOUSELEAVE) {
        gui.on_mouse_leave();
        return 0;
    }

    else if(message == WM_LBUTTONDOWN) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::left, true, position);
        return 0;
    }
    else if(message == WM_LBUTTONUP) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::left, false, position);
        return 0;
    }
    else if(message == WM_MBUTTONDOWN) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::middle, true, position);
        return 0;
    }
    else if(message == WM_MBUTTONUP) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::middle, false, position);
        return 0;
    }
    else if(message == WM_RBUTTONDOWN) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::right, true, position);
        return 0;
    }
    else if(message == WM_RBUTTONUP) {
        auto position = V2f { LOWORD(l_param), HIWORD(l_param) };
        gui.on_mouse_button(Mouse_Button::right, false, position);
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
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

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


    auto text = new Text_Def {};
    text->string = "hello, there!";
    text->font_face = &normal_font_face;
    text->size = 48.0f;
    text->color = V4f { 0.4f, 0.6f, 0.7f, 1.0f };

    auto left_rect = gui.create_widget<Rect_Widget>();
    left_rect->size = { 100.0f, 75.0f };
    left_rect->color = { 0.5f, 0.35f, 0.25f, 0.5f };

    auto right_rect = gui.create_widget<Rect_Widget>();
    right_rect->size = { 75.0f, 100.0f };
    right_rect->color = { 0.8f, 0.9f, 0.35f, 0.2f };

    auto text_edit = gui.create_widget<Simple_Line_Edit>();
    text_edit->font_face = &normal_font_face;

    auto button = new Simple_Button_Def();
    {
        auto button_text = new Text_Def();
        button_text->string = "Click me!";
        button_text->font_face = &normal_font_face;
        button_text->size = 16.0f;
        button_text->color = V4f { 1, 1, 1, 1 };

        auto padding = new Padding_Def();
        padding->child = button_text;
        padding->pad_min = { 5, 5 };
        padding->pad_max = { 5, 5 };

        button->child = padding;
        button->color = V4f { 0.29f, 0.56f, 0.89f, 1.0f };
    }

    auto other_button = new Simple_Button_Def();
    {
        auto button_text = new Text_Def();
        button_text->string = "Me too!";
        button_text->font_face = &normal_font_face;
        button_text->size = 16.0f;
        button_text->color = V4f { 1, 1, 1, 1 };

        auto padding = new Padding_Def();
        padding->child = button_text;
        padding->pad_min = { 5, 5 };
        padding->pad_max = { 5, 5 };

        other_button->child = padding;
        other_button->color = V4f { 0.29f, 0.56f, 0.89f, 1.0f };
    }

    auto stack = new Stack_Def();
    stack->children = {
        new Widget_Def(left_rect),
        text->with_key(new T_Key<Uint>(42)),
        new Widget_Def(right_rect),
        new Widget_Def(text_edit),
        button,
        other_button,
    };

    auto align = new Align_Def {};
    align->child = stack;
    align->align_point = { 0.5f, 0.5f };

    auto request_frame = [=]() { InvalidateRect(window, nullptr, false); };
    gui.create(align, request_frame);
    delete align;

    #if 0
    {
        auto text = new Text_Def {};
        text->string = "hi";
        text->font_face = &normal_font_face;
        text->size = 48.0f;
        text->color = { 1, 0, 1, 1 };

        auto middle_rect = gui.create_widget<Rect_Widget>();
        middle_rect->size = { 100.0f, 100.0f };
        middle_rect->color = { 0.5f, 0.3f, 0.9, 0.5f };

        auto stack = new Stack_Def();
        stack->children = {
            new Widget_Def(right_rect),
            new Widget_Def(middle_rect),
            text->with_key(new T_Key<Uint>(42)),
            new Widget_Def(left_rect),
            new Widget_Def(text_edit),
        };

        auto align = new Align_Def {};
        align->child = stack;
        align->align_point = { 0.25f, 0.5f };

        gui.set_root(align);
        delete align;
    }
    #endif


    //ShowWindow(window, SW_SHOWMAXIMIZED);
    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);


    auto msg = MSG {};
    while(GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    gui.destroy();
    printf("done.\n");

    return 0;
}


