/* Copyright © 2018-2020 N. Van Bossuyt.                                      */
/* This code is licensed under the MIT License.                               */
/* See: LICENSE.md                                                            */

#include <libgraphic/Font.h>
#include <libgraphic/Painter.h>
#include <libmath/math.h>
#include <libsystem/assert.h>

void painter_delete(Painter *);

Painter *painter_create(Bitmap *bitmap)
{
    Painter *painter = __create(Painter);

    painter->bitmap = bitmap;
    painter->cliprect = bitmap_bound(bitmap);
    painter->cliprect_stack_top = 0;

    return painter;
}

void painter_destroy(Painter *painter)
{
    free(painter);
}

void painter_push_cliprect(Painter *painter, Rectangle cliprect)
{
    assert(painter->cliprect_stack_top < 32);

    painter->cliprect_stack[painter->cliprect_stack_top] = painter->cliprect;
    painter->cliprect_stack_top++;

    painter->cliprect = rectangle_clip(painter->cliprect, cliprect);
}

void painter_pop_cliprect(Painter *painter)
{
    assert(painter->cliprect_stack_top > 0);

    painter->cliprect_stack_top--;
    painter->cliprect = painter->cliprect_stack[painter->cliprect_stack_top];
}

void painter_plot_pixel(Painter *painter, Point position, Color color)
{
    if (rectangle_containe_point(painter->cliprect, position))
    {
        bitmap_blend_pixel(painter->bitmap, position, color);
    }
}

void painter_blit_bitmap_fast(
    Painter *painter,
    Bitmap *bitmap,
    Rectangle source,
    Rectangle destination)
{
    for (int x = 0; x < destination.width; x++)
    {
        for (int y = 0; y < destination.height; y++)
        {
            Color sample = bitmap_get_pixel(bitmap, (Point){source.X + x, source.Y + y});
            painter_plot_pixel(painter, point_add(destination.position, (Point){x, y}), sample);
        }
    }
}

void painter_blit_bitmap_scaled(
    Painter *painter,
    Bitmap *bitmap,
    Rectangle source,
    Rectangle destination)
{
    for (int x = 0; x < destination.width; x++)
    {
        for (int y = 0; y < destination.height; y++)
        {
            float xx = x / (float)destination.width;
            float yy = y / (float)destination.height;

            Color sample = bitmap_sample(bitmap, source, xx, yy);
            painter_plot_pixel(painter, point_add(destination.position, (Point){x, y}), sample);
        }
    }
}

void painter_blit_bitmap(
    Painter *painter,
    Bitmap *bitmap,
    Rectangle source,
    Rectangle destination)
{
    if (source.width == destination.width &&
        source.height == destination.height)
    {
        painter_blit_bitmap_fast(painter, bitmap, source, destination);
    }
    else
    {
        painter_blit_bitmap_scaled(painter, bitmap, source, destination);
    }
}

void painter_clear(Painter *painter, Color color)
{
    painter_clear_rectangle(painter, bitmap_bound(painter->bitmap), color);
}

void painter_clear_rectangle(Painter *painter, Rectangle rect, Color color)
{
    Rectangle clipped = rectangle_clip(painter->cliprect, rect);

    if (rectangle_is_empty(clipped))
    {
        return;
    }

    for (int x = 0; x < clipped.width; x++)
    {
        for (int y = 0; y < clipped.height; y++)
        {
            bitmap_set_pixel(
                painter->bitmap,
                (Point){clipped.X + x, clipped.Y + y},
                color);
        }
    }
}

void painter_fill_rectangle(Painter *painter, Rectangle rect, Color color)
{
    Rectangle rect_absolue = rectangle_clip(painter->cliprect, rect);

    for (int xx = 0; xx < rect_absolue.width; xx++)
    {
        for (int yy = 0; yy < rect_absolue.height; yy++)
        {
            bitmap_blend_pixel(
                painter->bitmap,
                (Point){rect_absolue.X + xx, rect_absolue.Y + yy},
                color);
        }
    }
}

void painter_draw_line_x_aligned(Painter *painter, int x, int start, int end, Color color)
{
    for (int i = start; i < end; i++)
    {
        painter_plot_pixel(painter, (Point){x, i}, color);
    }
}

void painter_draw_line_y_aligned(Painter *painter, int y, int start, int end, Color color)
{
    for (int i = start; i < end; i++)
    {
        painter_plot_pixel(painter, (Point){i, y}, color);
    }
}

void painter_draw_line_not_aligned(Painter *painter, Point a, Point b, Color color)
{
    int dx = abs(b.X - a.X), sx = a.X < b.X ? 1 : -1;
    int dy = abs(b.Y - a.Y), sy = a.Y < b.Y ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2, e2;

    for (;;)
    {
        painter_plot_pixel(painter, a, color);

        if (a.X == b.X && a.Y == b.Y)
            break;

        e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            a.X += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            a.Y += sy;
        }
    }
}

void painter_draw_line(Painter *painter, Point a, Point b, Color color)
{
    if (a.X == b.X)
    {
        painter_draw_line_x_aligned(painter, a.X, MIN(a.Y, b.Y), MAX(a.Y, b.Y), color);
    }
    else if (a.Y == b.Y)
    {
        painter_draw_line_y_aligned(painter, a.Y, MIN(a.X, b.X), MAX(a.X, b.X), color);
    }
    else
    {
        painter_draw_line_not_aligned(painter, a, b, color);
    }
}

void painter_draw_rectangle(Painter *painter, Rectangle rect, Color color)
{
    painter_draw_line(painter, rect.position, point_sub(point_add(rect.position, point_x(rect.size)), (Point){1, 0}), color);
    painter_draw_line(painter, rect.position, point_sub(point_add(rect.position, point_y(rect.size)), (Point){0, 1}), color);
    painter_draw_line(painter, point_sub(point_add(rect.position, point_x(rect.size)), (Point){1, 0}), point_sub(point_add(rect.position, rect.size), (Point){1, 0}), color);
    painter_draw_line(painter, point_sub(point_add(rect.position, point_y(rect.size)), (Point){0, 1}), point_sub(point_add(rect.position, rect.size), (Point){0, 1}), color);
}

const int FONT_SIZE = 16;

void painter_blit_bitmap_colored(Painter *painter, Bitmap *src, Rectangle src_rect, Rectangle dst_rect, Color color)
{
    for (int x = 0; x < dst_rect.width; x++)
    {
        for (int y = 0; y < dst_rect.height; y++)
        {
            double xx = x / (double)dst_rect.width;
            double yy = y / (double)dst_rect.height;

            Color sample = bitmap_sample(src, src_rect, xx, yy);

            Color final = color;
            final.A = sample.R;

            painter_plot_pixel(painter, point_add(dst_rect.position, (Point){x, y}), final);
        }
    }
}

void painter_draw_glyph(Painter *painter, Font *font, Glyph *glyph, Point position, Color color)
{
    Rectangle dest;
    dest.position = point_sub(position, glyph->origin);
    dest.size = glyph->bound.size;

    painter_blit_bitmap_colored(painter, font->bitmap, glyph->bound, dest, color);
}