#include "elefont.h"

#include <inttypes.h>
#include <stdlib.h>

Font read_txt(FILE* src)
{
    uint8_t width;
    uint8_t height;
    check(fscanf(src, "%" SCNu8 " %" SCNu8, &width, &height) == 2, "Could not read width and height\n");
    
    Font font = new_af(width, height);
    
    uint8_t mask = 0;
    uint32_t bytes_to_encode = bits_in_bitmap(&font);
    int32_t bitmap_index = -1;

    while (bytes_to_encode--)
    {
        bool bit;
        while (true)
        {
            check(!feof(src), "Unexpected end of src.\n");

            int c = fgetc(src);
            if (c == '0')
            {
                bit = false;
                break;
            }
            else if (c == '1')
            {
                bit = true;
                break;
            }
        }
        
        if (!mask)
        {
            mask = 0x80;
            bitmap_index++;
        }

        if (bit)
            font.bitmap[bitmap_index] |= mask;

        mask >>= 1;
    }

    return font;
}

void write_txt(FILE* dst, const Font* src)
{
    fprintf(dst, "%" PRIu8 " %" PRIu8 "\n\n", src->width, src->height);

    uint8_t w = 0;
    uint8_t h = 0;
    uint32_t bits_to_decode = bits_in_bitmap(src);
    
    uint8_t mask = 0x80;
    const uint8_t* current = src->bitmap;
    while (bits_to_decode--)
    {
        fputc((*current & mask) ? '1' : '0', dst);
        if (++w == src->width)
        {
            w = 0;
            fputc('\n', dst);
            
            if (++h == src->height)
            {
                h = 0;
                fputc('\n', dst);
            }
        }
        
        mask >>= 1;
        if (!mask)
        {
            mask = 0x80;
            current++;
        }
    }
}
