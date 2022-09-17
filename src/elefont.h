#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CHARACTERS_IN_FONT 94

// A monospaced, monochrome and ASCII only font format.
// Fonts encode ! to ~ (it is assumed that space is a 
// blank character) in a bit array starting from the
// top left and ending in the lower right.
//
// Sample of a 2x3 font bitmap:
// !!!!!!"" """"#### ##$$$$$$ ''''''((
// (((()))) ))****** and so on. 
typedef struct 
{
    uint8_t* bitmap;
    uint8_t width;
    uint8_t height;
} Font;

uint32_t bits_in_bitmap(const Font* font);
uint32_t bytes_in_bitmap(const Font* font);

Font new_af(uint8_t width, uint8_t height);

bool read_af_bit(const Font* font, char c, uint8_t x, uint8_t y);
void write_af_bit(Font* font, char c, uint8_t x, uint8_t y, bool bit);

typedef Font (*Reader)(FILE* src);
typedef void (*Writer)(FILE* dst, const Font* src);

Font read_af(FILE* src);
void write_af(FILE* dst, const Font* src);
Font read_bdf(FILE* src);
void write_bdf(FILE* dst, const Font* src);
void write_c(FILE* dst, const Font* src);
Font read_txt(FILE* src);
void write_txt(FILE* dst, const Font* src);

void check(bool assertion, const char* format, ...);