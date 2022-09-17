#include "elefont.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct
{
    int32_t fbbx;
    int32_t fbby;
    int32_t xoff;
    int32_t yoff;
} Font_Bounding_Box;

typedef struct
{
    int32_t bbw;
    int32_t bbh;
    int32_t bbxoff0x;
    int32_t bbyoff0y;
} Bbx;

typedef struct
{
    Font_Bounding_Box fbb;
    int32_t metrics_set;
} Header;

static const char* read_line(FILE* src)
{
    static char line[2000];
    uint16_t length = 0;

    while (!feof(src))
    {
        int c = fgetc(src);
        if (c == '\n')
        {
            line[length] = 0;
            break;
        }
        else if (c != '\r')
            line[length++] = (char)c;
    }

    return (char*)line;
}

static bool parse_line(const char* line, const char* format, ...)
{
    int arguments_length = 0;
    for (int i = 0; format[i]; i++)
    {
        if (format[i] == '%')
            arguments_length++;
    }

    if (arguments_length)
    {
        va_list arguments;
        va_start(arguments, format);
        int res = vsscanf(line, format, arguments);
        va_end(arguments);

        return res == arguments_length;
    }
    else
        return !strcmp(line, format);
}

static bool starts_with(const char* line, const char* string)
{
    return !strncmp(line, string, strlen(string));
}

static bool parse_size(const char* line)
{
    return starts_with(line, "SIZE ");
}

static bool parse_fbb(const char* line, Font_Bounding_Box* fbb)
{
    return parse_line(line, "FONTBOUNDINGBOX %" SCNi32 " %" SCNi32 " %" SCNi32 " %" SCNi32 "\n", &fbb->fbbx, &fbb->fbby, &fbb->xoff, &fbb->yoff);
}

static bool parse_metrics_set(const char* line, int32_t* metrics_set)
{
    return parse_line(line, "METRICSSET %" SCNi32, metrics_set);
}

static bool parse_d_width(const char* line)
{
    return starts_with(line, "DWIDTH ");
}

static bool parse_s_width(const char* line)
{
    return starts_with(line, "SWIDTH ");
}

static bool parse_encoding(const char* line, char* c)
{
    int32_t code_point;
    if (parse_line(line, "ENCODING %" SCNi32, &code_point) && code_point >= '!' && code_point <= '~')
    {
        *c = (char)code_point;
        return true;
    }

    return false;
}

static bool parse_bbx(const char* line, Bbx* bbx)
{
    return parse_line(line, "BBX %" SCNi32 " %" SCNi32 " %" SCNi32 " %" SCNi32 "\n", &bbx->bbw, &bbx->bbh, &bbx->bbxoff0x, &bbx->bbyoff0y);
}

static bool parse_byte(const char* line, size_t* index, uint8_t* byte)
{
    if (sscanf(&line[*index], "%" SCNx8, byte) != 1)
        return false;

    while (line[*index] && line[*index] != ' ')
        (*index)++;

    while (line[*index] && line[*index] == ' ')
        (*index)++;

    return true;
}

Font read_bdf(FILE* src)
{
    const char* line = read_line(src);
    check(parse_line(line, "STARTFONT 2.1"), "Only BDF version 2.1 is supported.\n");

    Header header = { 0 };
    while (!feof(src))
    {
        line = read_line(src);
        if (parse_line(line, "CHARS %" SCNi32, &(int32_t){0}))
            break;

        if (parse_size(line) ||
            parse_fbb(line, &header.fbb) ||
            parse_metrics_set(line, &header.metrics_set) ||
            parse_d_width(line) ||
            parse_s_width(line))
            continue;
    }

    // This way I only have to deal with DWIDTH if even that.
    check(!header.metrics_set, "Only METRICSET 0 is supported.");
    
    Font font = new_af((uint8_t)header.fbb.fbbx, (uint8_t)header.fbb.fbby);
    while (!feof(src))
    {
        char c;
        line = read_line(src);
        if (!parse_encoding(line, &c))
            continue;
        
        // SWIDTH and DWIDTH can be defined globally and then overwritten by each character.
        line = read_line(src);
        if (parse_s_width(line))
            line = read_line(src);
        
        if (parse_d_width(line))
            line = read_line(src);
        
        Bbx bbx;
        check(parse_bbx(line, &bbx), "BBX expected for character %c.\n", c);

        line = read_line(src);
        check(parse_line(line, "BITMAP"), "BITMAP expected for character %c.\n", c);

        uint8_t y = (uint8_t)(header.fbb.fbby + header.fbb.yoff - (bbx.bbyoff0y + bbx.bbh));
        while (!feof(src))
        {
            line = read_line(src);
            if (parse_line(line, "ENDCHAR"))
                break;

            size_t line_index = 0;
            uint8_t x = (uint8_t)bbx.bbxoff0x;

            uint8_t byte;
            while (parse_byte(line, &line_index, &byte))
            {
                uint8_t mask = 0x80;
                while (mask && x < bbx.bbxoff0x + bbx.bbw)
                {
                    write_af_bit(&font, c, x++, y, mask & byte);
                    mask >>= 1;
                }
            }

            y++;
        }
    }

    return font;
}