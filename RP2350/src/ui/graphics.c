#include "coinflip_graphics.h"

#include "fonts.h"

#include <stddef.h>
#include <string.h>

static void set_pixel( CoinflipCanvas *canvas, uint16_t x, uint16_t y, uint16_t color )
{
    if( canvas == NULL || canvas->pixels == NULL || x >= canvas->width || y >= canvas->height )
        return;

    canvas->pixels[ ( uint32_t )y * canvas->width + x ] = color;
}

void coinflip_graphics_clear( CoinflipCanvas *canvas, uint16_t color )
{
    if( canvas == NULL || canvas->pixels == NULL )
        return;

    for( uint32_t pixel = 0; pixel < ( uint32_t )canvas->width * canvas->height; ++pixel )
        canvas->pixels[ pixel ] = color;
}

void coinflip_graphics_fill_rect( CoinflipCanvas *canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color )
{
    if( canvas == NULL || x >= canvas->width || y >= canvas->height )
        return;

    uint32_t x_end = ( uint32_t )x + width;
    uint32_t y_end = ( uint32_t )y + height;

    if( x_end > canvas->width )
        x_end = canvas->width;

    if( y_end > canvas->height )
        y_end = canvas->height;

    for( uint16_t row = y; row < y_end; ++row )
    {
        for( uint16_t column = x; column < x_end; ++column )
            set_pixel( canvas, column, row, color );
    }
}

void coinflip_graphics_draw_rect( CoinflipCanvas *canvas, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color )
{
    if( width == 0U || height == 0U )
        return;

    coinflip_graphics_fill_rect( canvas, x, y, width, 1, color );
    coinflip_graphics_fill_rect( canvas, x, ( uint16_t )( y + height - 1U ), width, 1, color );
    coinflip_graphics_fill_rect( canvas, x, y, 1, height, color );
    coinflip_graphics_fill_rect( canvas, ( uint16_t )( x + width - 1U ), y, 1, height, color );
}

static void draw_character( CoinflipCanvas *canvas, const sFONT *font, uint16_t x, uint16_t y, char character, uint8_t scale, uint16_t foreground, uint16_t background )
{
    const uint16_t bytes_per_row = ( uint16_t )( ( font->Width + 7U ) / 8U );
    const size_t glyph_size = ( size_t )font->Height * bytes_per_row;
    const uint8_t *glyph;

    if( character < ' ' || character > '~' )
        character = '?';

    glyph = font->table + ( size_t )( character - ' ' ) * glyph_size;

    for( uint16_t row = 0; row < font->Height; ++row )
    {
        for( uint16_t column = 0; column < font->Width; ++column )
        {
            const uint8_t bits = glyph[ row * bytes_per_row + column / 8U ];
            const uint16_t color = ( bits & ( 0x80U >> ( column % 8U ) ) )
                                   ? foreground : background;

            coinflip_graphics_fill_rect( canvas, ( uint16_t )( x + column * scale ), ( uint16_t )( y + row * scale ), scale, scale, color );
        }
    }
}

void coinflip_graphics_text( CoinflipCanvas *canvas, uint16_t x, uint16_t y, const char *text, uint8_t scale, uint16_t foreground, uint16_t background )
{
    if( text == NULL || scale == 0U )
        return;

    while( *text != '\0' )
    {
        draw_character( canvas, &Font16, x, y, *text, scale, foreground, background );

        x = ( uint16_t )( x + Font16.Width * scale );
        ++text;
    }
}

void coinflip_graphics_text20( CoinflipCanvas *canvas, uint16_t x, uint16_t y, const char *text, uint16_t foreground, uint16_t background )
{
    if( text == NULL )
        return;

    while( *text != '\0' )
    {
        draw_character( canvas, &Font20, x, y, *text, 1, foreground, background );

        x = ( uint16_t )( x + Font20.Width );
        ++text;
    }
}

void coinflip_graphics_text24( CoinflipCanvas *canvas, uint16_t x, uint16_t y, const char *text, uint16_t foreground, uint16_t background )
{
    if( text == NULL )
        return;

    while( *text != '\0' )
    {
        draw_character( canvas, &Font24, x, y, *text, 1, foreground, background );

        x = ( uint16_t )( x + Font24.Width );
        ++text;
    }
}

void coinflip_graphics_text24_centered( CoinflipCanvas *canvas, uint16_t y, const char *text, uint16_t foreground, uint16_t background )
{
    size_t width;
    uint16_t x;

    if( canvas == NULL || text == NULL )
        return;

    width = strlen( text ) * Font24.Width;
    x = width < canvas->width ? ( uint16_t )( ( canvas->width - width ) / 2U ) : 0U;

    coinflip_graphics_text24( canvas, x, y, text, foreground, background );
}

void coinflip_graphics_text_centered( CoinflipCanvas *canvas, uint16_t y, const char *text, uint8_t scale, uint16_t foreground, uint16_t background )
{
    size_t width;
    uint16_t x;

    if( canvas == NULL || text == NULL || scale == 0U )
        return;

    width = strlen( text ) * Font16.Width * scale;
    x = width < canvas->width ? ( uint16_t )( ( canvas->width - width ) / 2U ) : 0U;

    coinflip_graphics_text( canvas, x, y, text, scale, foreground, background );
}
