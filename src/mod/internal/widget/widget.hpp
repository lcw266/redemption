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
#pragma once

#include "utils/rect.hpp"
#include "utils/colors.hpp"
#include "utils/sugar/zstring_view.hpp"
#include "core/callback.hpp"


struct Keymap;
namespace gdi
{
    class GraphicApi;
    class CachePointerIndex;
} // namespace gdi


class WidgetTooltipShower
{
public:
    WidgetTooltipShower() = default;

    WidgetTooltipShower(WidgetTooltipShower const&) = delete;
    WidgetTooltipShower& operator=(WidgetTooltipShower const&) = delete;

    virtual ~WidgetTooltipShower() = default;

    virtual void show_tooltip(
        const char * text, int x, int y,
        Rect preferred_display_rect,
        Rect mouse_area) = 0;

    void hide_tooltip()
    {
        this->show_tooltip(nullptr, 0, 0, Rect(), Rect());
    }
};


class Widget : public RdpInput
{
public:
    struct Color
    {
        constexpr Color(BGRColor color) noexcept
        : rdp_color_(encode_color24()(color))
        {}

        constexpr Color(NamedBGRColor color) noexcept
        : rdp_color_(encode_color24()(color))
        {}

        constexpr Color(BGRasRGBColor const & color) noexcept
        : rdp_color_(encode_color24()(color))
        {}

        constexpr explicit Color(uint32_t color = 0) noexcept /*NOLINT*/
        : rdp_color_(RDPColor::from(color))
        {}

        constexpr RDPColor as_rdp_color() const noexcept { return rdp_color_; }
        constexpr BGRColor as_bgr() const noexcept { return rdp_color_.as_bgr(); }
        constexpr BGRasRGBColor as_rgb() const noexcept { return rdp_color_.as_rgb(); }

        constexpr operator RDPColor () const noexcept { return rdp_color_; }
        constexpr operator BGRColor () const noexcept { return rdp_color_.as_bgr(); }
        constexpr operator BGRasRGBColor () const noexcept { return rdp_color_.as_rgb(); }

    private:
        RDPColor rdp_color_;
    };

    enum class Focusable : bool
    {
        No,
        Yes,
    };

    enum class PointerType : uint8_t
    {
        Custom,
        Normal,
        Edit,
    };

protected:
    gdi::GraphicApi & drawable;

private:
    Rect rect;

public:
    Focusable focusable;
    PointerType pointer_flag;
    bool has_focus;

public:
    Widget(gdi::GraphicApi & drawable, Focusable focusable)
    : drawable(drawable)
    , focusable(focusable)
    , pointer_flag(PointerType::Normal)
    , has_focus(false)
    {}

    void set_focusable(Focusable is_focusable = Focusable::Yes)
    {
        focusable = is_focusable;
    }

    void set_unfocusable()
    {
        focusable = Focusable::No;
    }

    virtual bool next_focus()
    {
        return false;
    }

    virtual bool previous_focus()
    {
        return false;
    }

    Widget * last_widget_at_pos(int16_t x, int16_t y)
    {
        Widget * w = this;
        int count = 10;
        while (w->widget_at_pos(x, y)
               && (w != w->widget_at_pos(x, y))
               && (--count > 0)) {
            w = w->widget_at_pos(x, y);
        }
        return w;
    }

    void rdp_gdi_up_and_running() override
    {}

    void rdp_gdi_down() override
    {}

    void rdp_input_scancode(KbdFlags flags, Scancode scancode, uint32_t event_time, Keymap const& keymap) override {
        (void)flags;
        (void)scancode;
        (void)event_time;
        (void)keymap;
    }

    void rdp_input_unicode(KbdFlags flag, uint16_t unicode) override
    {
        (void)unicode;
        (void)flag;
    }

    // - mouve event (mouse moves or a button went up or down)
    void rdp_input_mouse(uint16_t device_flags, uint16_t x, uint16_t y) override
    {
        (void)device_flags;
        (void)x;
        (void)y;
    }

    // - synchronisation of capslock, numlock, etc state.
    void rdp_input_synchronize(KeyLocks locks) override
    {
        (void)locks;
    }

    virtual Widget * widget_at_pos(int16_t x, int16_t y)
    {
        return (this->rect.contains_pt(x, y)) ? this : nullptr;
    }

    virtual void set_xy(int16_t x, int16_t y)
    {
        this->rect.x = x;
        this->rect.y = y;
    }

    virtual void set_wh(uint16_t w, uint16_t h)
    {
        this->rect.cx = w;
        this->rect.cy = h;
    }

    void set_wh(Dimension dim)
    {
        this->set_wh(dim.w, dim.h);
    }

    virtual void set_color(Color bg_color, Color fg_color)
    {
        (void)bg_color;
        (void)fg_color;
    }

    virtual void move_xy(int16_t x, int16_t y)
    {
        this->set_xy(this->rect.x + x, this->rect.y + y);
    }

    virtual void init_focus()
    {
        if (this->focusable == Focusable::Yes) {
            this->has_focus = true;
        }
    }

    enum {
          focus_reason_tabkey
        , focus_reason_backtabkey
        , focus_reason_mousebutton1
    };
    virtual void focus(int reason)
    {
        (void)reason;
        if (!this->has_focus){
            this->has_focus = true;
            this->rdp_input_invalidate(this->rect);
        }
    }

    virtual void blur()
    {
        if (this->has_focus){
            this->has_focus = false;
            this->rdp_input_invalidate(this->rect);
        }
    }

    virtual Dimension get_optimal_dim() const
    {
        return Dimension(0, 0);
    }

    virtual void clipboard_insert_utf8(zstring_view text)
    {
        (void)text;
    }

    ///Return x position in it's screen
    [[nodiscard]] int16_t x() const
    {
        return this->rect.x;
    }

    ///Return y position in it's screen
    [[nodiscard]] int16_t y() const
    {
        return this->rect.y;
    }

    ///Return width
    [[nodiscard]] uint16_t cx() const
    {
        return this->rect.cx;
    }

    ///Return height
    [[nodiscard]] uint16_t cy() const
    {
        return this->rect.cy;
    }

    ///Return x()+cx()
    [[nodiscard]] int16_t eright() const
    {
        return this->rect.eright();
    }

    ///Return y()+cy()
    [[nodiscard]] int16_t ebottom() const
    {
        return this->rect.ebottom();
    }

    [[nodiscard]] Rect get_rect() const {
        return this->rect;
    }

    [[nodiscard]] virtual gdi::CachePointerIndex const* get_cache_pointer_index() const
    {
        return nullptr;
    }
};
