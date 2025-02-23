/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *   Product name: redemption, a FLOSS RDP proxy
 *   Copyright (C) Wallix 2010-2012
 *   Author(s): Christophe Grosjean, Dominique Lafages, Jonathan Poelen,
 *              Meng Tan
 */

#include "core/RDP/orders/RDPOrdersPrimaryOpaqueRect.hpp"
#include "core/font.hpp"
#include "mod/internal/widget/label.hpp"
#include "mod/internal/copy_paste.hpp"
#include "gdi/graphic_api.hpp"
#include "gdi/text_metrics.hpp"
#include "utils/sugar/cast.hpp"
#include "utils/utf.hpp"

WidgetLabel::WidgetLabel(
    gdi::GraphicApi & drawable, chars_view text,
    Color fgcolor, Color bgcolor, Font const & font,
    int xtext, int ytext
)
    : Widget(drawable, Focusable::No)
    , x_text(xtext)
    , y_text(ytext)
    , bg_color(bgcolor)
    , fg_color(fgcolor)
    , w_border(x_text)
    , font(font)
{
    this->set_text(text);
}

WidgetLabel::WidgetLabel(
    gdi::GraphicApi & drawable, char const* text,
    Color fgcolor, Color bgcolor, Font const & font,
    int xtext, int ytext
)
: WidgetLabel(drawable, {text, text ? strlen(text) : 0u},
              fgcolor, bgcolor, font, xtext, ytext)
{}

void WidgetLabel::set_text(char const* text)
{
    this->set_text({text, text ? strlen(text) : 0u});
}

void WidgetLabel::set_text(chars_view text)
{
    this->buffer[0] = 0;
    if (!text.empty()) {
        const size_t remain_n = buffer_size - 1;
        const size_t max = ((remain_n >= text.size()) ? text.size() :
                            ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text.data()), remain_n));
        memcpy(this->buffer, text.data(), max);
        this->buffer[max] = 0;
    }
}

const char * WidgetLabel::get_text() const
{
    return this->buffer;
}

void WidgetLabel::rdp_input_invalidate(Rect clip)
{
    Rect rect_intersect = clip.intersect(this->get_rect());

    if (!rect_intersect.isempty()) {
        this->draw(
            rect_intersect, this->get_rect(), this->drawable, this->buffer,
            this->fg_color, this->bg_color, gdi::ColorCtx::depth24(),
            this->font, this->x_text, this->y_text);
    }
}

void WidgetLabel::draw(
    Rect const clip, Rect const rect, gdi::GraphicApi& drawable,
    char const* text, Color fgcolor, Color bgcolor, gdi::ColorCtx color_ctx,
    Font const & font, int xtext, int ytext)
{
    drawable.draw(RDPOpaqueRect(rect, bgcolor), clip, color_ctx);
    gdi::server_draw_text(
        drawable, font,
        xtext + rect.x, ytext + rect.y,
        text, fgcolor, bgcolor,
        color_ctx, rect.intersect(clip)
    );
}

Dimension WidgetLabel::get_optimal_dim() const
{
    int width = this->buffer[0] ? gdi::TextMetrics(this->font, this->buffer).width : 0;
    return Dimension(width + this->x_text * 2, this->font.max_height() + this->y_text * 2);
}

Dimension WidgetLabel::get_optimal_dim(Font const & font, char const* text, int xtext, int ytext)
{
    char buffer[buffer_size];

    buffer[0] = 0;
    if (text) {
        const size_t remain_n = buffer_size - 1;
        const size_t n = strlen(text);
        const size_t max = ((remain_n >= n) ? n :
                            ::UTF8StringAdjustedNbBytes(::byte_ptr_cast(text), remain_n));
        memcpy(buffer, text, max);
        buffer[max] = 0;
    }

    int width = buffer[0] ? gdi::TextMetrics(font, buffer).width : 0;
    return Dimension(width + xtext * 2, font.max_height() + ytext * 2);
}

bool WidgetLabel::shift_text(int pos_x)
{
    bool res = true;
    if (pos_x + this->x_text > this->cx() - 4) {
        this->x_text = this->cx() - pos_x - 4;
    }
    else if (pos_x + this->x_text < this->w_border) {
        this->x_text = this->w_border - pos_x;
    }
    else {
        res = false;
    }
    return res;
}

void WidgetLabel::set_color(Color bg_color, Color fg_color)
{
    this->bg_color = bg_color;
    this->fg_color = fg_color;
}

void WidgetLabel::auto_resize()
{
    Dimension dim = this->get_optimal_dim();
    this->set_wh(dim);
}
