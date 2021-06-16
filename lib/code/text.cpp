#include <cpp-gui/text.hpp>

void Font_Face::create(IDWriteFontFace* dwrite_font_face) {
    this->dwrite_font_face = dwrite_font_face;

    dwrite_font_face->GetMetrics(&this->font_metrics);

    for(Uint i = 0; i < 256; i += 1) {
        auto code_point = (Uint32)i;

        auto hr = HRESULT {};

        auto glyph_index = Uint16 {};
        hr = dwrite_font_face->GetGlyphIndices(&code_point, 1, &glyph_index);
        assert(SUCCEEDED(hr));

        auto glyph_metrics = DWRITE_GLYPH_METRICS {};
        hr = dwrite_font_face->GetDesignGlyphMetrics(&glyph_index, 1, &glyph_metrics);
        assert(SUCCEEDED(hr));

        this->glyph_indices[i] = glyph_index;
        this->glyph_metrics[i] = glyph_metrics;
    }
}



void Text_Layout::create(Font_Face* font_face, Float32 size, const std::string& string) {
    auto glyph_count = (Uint32)string.size();

    auto glyph_indices  = new Uint16[glyph_count];
    auto glyph_advances = new Float32[glyph_count];
    auto glyph_offsets  = new DWRITE_GLYPH_OFFSET[glyph_count];

    auto scale = font_face->scale(size);

    auto cursor = (Float32)0;

    for(size_t i = 0; i < glyph_count; i += 1) {
        auto code_point  = string[i];
        auto glyph_index = font_face->glyph_indices[code_point];

        auto base_advance = font_face->glyph_metrics[code_point].advanceWidth;
        auto advance = roundf(scale * base_advance);

        glyph_indices[i]  = glyph_index;
        glyph_advances[i] = advance;
        glyph_offsets[i]  = { 0.0f, 0.0f };

        cursor += advance;
    }

    this->font_face         = font_face;
    this->size.x            = cursor;
    this->size.y            = ceil(font_face->line_height(size));
    this->run.fontFace      = font_face->dwrite_font_face;
    this->run.fontEmSize    = size;
    this->run.glyphCount    = glyph_count;
    this->run.glyphIndices  = glyph_indices;
    this->run.glyphAdvances = glyph_advances;
    this->run.glyphOffsets  = glyph_offsets;
}

void Text_Layout::destroy() {
    if(this->run.glyphIndices != nullptr) {
        delete[] this->run.glyphIndices;
    }
    if(this->run.glyphAdvances != nullptr) {
        delete[] this->run.glyphAdvances;
    }
    if(this->run.glyphOffsets != nullptr) {
        delete[] this->run.glyphOffsets;
    }
    *this = {};
}

void Text_Layout::paint(ID2D1RenderTarget* target, V2f position, ID2D1Brush* brush) const {
    position.y += round(this->font_face->ascent(this->run.fontEmSize));
    target->DrawGlyphRun(
        to_d2d_point2f(position),
        &this->run,
        brush,
        DWRITE_MEASURING_MODE_NATURAL
    );
}

void Text_Layout::paint(ID2D1RenderTarget* target, V2f position, V4f color) const {
    auto brush = (ID2D1SolidColorBrush*)nullptr;
    auto hr = target->CreateSolidColorBrush(to_d2d_colorf(color), &brush);
    assert(SUCCEEDED(hr));
    defer { brush->Release(); };

    this->paint(target, position, brush);
}

void Text_Layout::paint(ID2D1RenderTarget* target, V4f color) const {
    this->paint(target, V2f { 0, 0 }, color);
}
