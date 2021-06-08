#pragma once

#include "common.hpp"


struct Font_Face {
    IDWriteFontFace* dwrite_font_face;
    DWRITE_FONT_METRICS font_metrics;
    DWRITE_GLYPH_METRICS glyph_metrics[256];
    Uint16 glyph_indices[256];


    void create(IDWriteFontFace* dwrite_font_face);
    void destroy();


    Float32 scale(Float32 size) const {
        return size / this->font_metrics.designUnitsPerEm;
    }

    Float32 line_height(Float32 size) const {
        return this->scale(size) * (this->font_metrics.ascent + this->font_metrics.descent);
    }

    Float32 ascent(Float32 size) const {
        return this->scale(size) * this->font_metrics.ascent;
    }

    Float32 descent(Float32 size) const {
        return this->scale(size) * this->font_metrics.descent;
    }

    Float32 x_width(Float32 size) const {
        return this->scale(size) * this->glyph_metrics['x'].advanceWidth;
    }

    Float32 x_height(Float32 size) const {
        return this->scale(size) * this->font_metrics.xHeight;
    }
};


struct Text_Layout {
    Font_Face* font_face;
    DWRITE_GLYPH_RUN run;
    V2f size;


    void create(Font_Face* font_face, Float32 size, const std::string& string);

    void destroy();

    void paint(ID2D1RenderTarget* target, V2f position, ID2D1Brush* brush) const;

    void paint(ID2D1RenderTarget* target, V2f position, V4f color) const;

    void paint(ID2D1RenderTarget* target, V4f color) const;
};

