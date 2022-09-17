#include "elefont.h"

#include <stdio.h>
#include <stdlib.h>

Font new_af(uint8_t width, uint8_t height)
{
    Font font = { .width = width, .height = height };
    
    font.bitmap = calloc(bytes_in_bitmap(&font), 1);
    check(font.bitmap, "Could not allocate memory for the bitmap.\n");
    
    return font;
}

static void read_field(FILE* src, const char* field_name, size_t field_size, void* field)
{
    size_t bytes_read = fread(field, 1, field_size, src);
    check(bytes_read == field_size, "%s: Read %zu bytes but expected %zu\n", field_name, bytes_read, field_size);
}

static void read_u8_field(FILE* src, const char* field_name, uint8_t* field)
{
    read_field(src, field_name, sizeof(uint8_t), field);
}

Font read_af(FILE* src)
{
    Font font;
    read_u8_field(src, "Width",  &font.width);
    read_u8_field(src, "Height", &font.height);

    uint32_t bytes_to_read = bytes_in_bitmap(&font);
    font.bitmap = malloc(bytes_to_read);

    read_field(src, "Bitmap", bytes_to_read, font.bitmap);
    
    return font;
}

static void write_field(FILE* dest, const char* field_name, size_t field_size, void* field)
{
    size_t bytes_written = fwrite(field, 1, field_size, dest);
    check(bytes_written == field_size, "%s: Wrote %zu bytes but expected %zu\n", field_name, bytes_written, field_size);
}

static void write_u8_field(FILE* dest, const char* field_name, uint8_t field)
{
    write_field(dest, field_name, sizeof(field), &field);
}

void write_af(FILE* dest, const Font* font)
{
    write_u8_field(dest, "Width",  font->width);
    write_u8_field(dest, "Height", font->height);
    write_field(   dest, "Bitmap", bytes_in_bitmap(font), font->bitmap);
}

static uint32_t bits_in_character(const Font* font)
{
    return (uint32_t)font->width * (uint32_t)font->height;
}

uint32_t bits_in_bitmap(const Font* font)
{
    return bits_in_character(font) * CHARACTERS_IN_FONT;
}

uint32_t bytes_in_bitmap(const Font* font)
{
    uint32_t bits = bits_in_bitmap(font);
    return bits / 8 + (bits % 8 ? 1 : 0);
}

typedef struct
{
    uint32_t offset;
    uint8_t mask;
} Offset_Mask_Pair;

Offset_Mask_Pair find_ofp(const Font* font, char c, uint8_t x, uint8_t y)
{
    uint32_t bit_index = bits_in_character(font) * (c - '!') + font->width * y + x;
    return (Offset_Mask_Pair)
    {
        .offset =        bit_index / 8,
        .mask = 0x80 >> (bit_index % 8)
    };
}

bool read_af_bit(const Font* font, char c, uint8_t x, uint8_t y)
{
    Offset_Mask_Pair ofp = find_ofp(font, c, x, y);
    return font->bitmap[ofp.offset] & ofp.mask;
}

void write_af_bit(Font* font, char c, uint8_t x, uint8_t y, bool bit)
{
    Offset_Mask_Pair ofp = find_ofp(font, c, x, y);
    if (bit)
        font->bitmap[ofp.offset] |= ofp.mask;
    else
        font->bitmap[ofp.offset] &= ~ofp.mask;
}
