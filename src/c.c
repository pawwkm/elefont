#include "elefont.h"

void write_c(FILE* dst, const Font* font)
{
    uint32_t size = bytes_in_bitmap(font);
    fprintf(dst, "#define CHARACTERS_IN_FONT %u\n"
                 "\n"
                 "typedef struct\n"
                 "{\n"
                 "    uint8_t* bitmap;\n"
                 "    uint8_t width;\n"
                 "    uint8_t height;\n"
                 "} Font;\n"
                 "\n"
                 "const Font font = \n"
                 "{\n"
                 "    .bitmap = (uint8_t[%u]) { ",
                 CHARACTERS_IN_FONT,
                 size);

    for (uint32_t i = 0; i < size; i++)
    {
        if (i)
            fprintf(dst, ", ");
        
        fprintf(dst, "%u", font->bitmap[i]);
    }

    fprintf(dst, " },\n"
                 "    .width = %u,\n"
                 "    .height = %u\n"
                 "};\n",
                 font->width, font->height);
}
