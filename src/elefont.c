#include "elefont.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

void check(bool assertion, const char* format, ...)
{
    if (assertion)
        return;
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    exit(EXIT_FAILURE);
}

static void usage(bool failure) 
{
    printf("elefont src [dst] | command\n"
           "     src    Path to the file to convert or print as the .txt format if dst is omitted.\n"
           "            .bdf, .txt and .af files are supported.\n"
           "     [dst]  The path to store the font at. .txt, .af and .c files are supported.\n"
           "\n"
           "     commands\n"
           "         new       Creates a new font.\n"
           "           width   The width of the font in pixels. From 1 to 255.\n"
           "           height  The height of the font in pixels. From 1 to 255.\n"
           "           [dst]   The path to store the font at.\n"
           "\n"
           "         resize    Add and/or subtract rows and columns of pixels.\n"
           "           t       Prepends t rows to the top.\n"
           "           b       Appends b rows to the bottom.\n"
           "           l       Prepends l columns to the left.\n"
           "           r       Appends r columns to the right.\n"
           "           src     The path to the font to resize.\n"
           "           [dst]   The path to store the font at.\n"
           " \n"
           "         -h --help Displays this message.\n");

    if (failure)
        exit(EXIT_FAILURE);
}

static FILE* open_file(const char* path, const char* mode)
{
    FILE* file = fopen(path, mode);
    if (!file)
    {
        fprintf(stderr, "Could not open %s because: %s\n", path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    return file;
}

static const char* extension_of(const char* path)
{
    size_t length = strlen(path);
    for (size_t i = length - 1; i > 0; i--)
    {
        if (path[i] == '.')
            return &path[i + 1];
    }
    
    return "No extension";
}

static Font read_font(const char* src)
{    
    static const struct
    {
        char* extension;
        Reader reader;
    } readers[] = 
    {
        { "af",  read_af  },
        { "bdf", read_bdf },
        { "txt", read_txt }
    };
    
    const char* extension = extension_of(src);
    Reader read = NULL;
    
    for (uint8_t i = 0; i < sizeof(readers) / sizeof(readers[0]); i++)
    {
        if (!strcmp(readers[i].extension, extension))
        {
            read = readers[i].reader;
            break;
        }
    }
    
    check(read, "%s files are not supported for reading.", extension);

    FILE* file = open_file(src, "rb");
    Font font = read(file);
    
    fclose(file);
    
    return font;
}

static void write_font(const char* dst, const Font* src)
{
    static const struct
    {
        char* extension;
        Writer writer;
    } writers[] = 
    {
        { "af",  write_af  },
        { "c",   write_c   },
        { "txt", write_txt }
    };
    
    const char* extension = dst ? extension_of(dst) : "txt";
    Writer write = NULL;
    
    for (uint8_t i = 0; i < sizeof(writers) / sizeof(writers[0]); i++)
    {
        if (!strcmp(writers[i].extension, extension))
        {
            write = writers[i].writer;
            break;
        }
    }
    
    check(write, "%s files are not supported for writing.", extension);
    FILE* file = dst ? open_file(dst, "wb") : stdout;
    
    write(file, src);
    if (dst)
        fclose(file);
}

static void new(const char* path, uint8_t width, uint8_t height)
{
    Font font = new_af(width, height);
    write_font(path, &font);
}

static Font resize(const char* src, int t, int b, int l, int r)
{
    Font og = read_font(src);
    int width = l + r + og.width;
    int height = t + b + og.height;

    check(width  > 0 && width  < 256, "Resized width is out of bounds.\n");
    check(height > 0 && height < 256, "Resized height is out of bounds.\n");

    Font font = new_af((uint8_t)width, (uint8_t)height);
    for (char c = '!'; c <= '~'; c++)
    {
        for (int h = 0; h < og.height; h++)
        {
            for (int w = 0; w < og.width; w++)
            {
                int x = w + l;
                int y = h + t;
                if (x >= 0 && x < font.width && y >= 0 && y < font.height)
                {
                    bool bit = read_af_bit(&og, c, (uint8_t)w, (uint8_t)h);
                    write_af_bit(&font, c, (uint8_t)x, (uint8_t)y, bit);
                }
            }
        }
    }

    return font;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
        usage(true);
    else if (argc == 2 || argc == 3)
    {
        Font src = read_font(argv[1]);
        write_font(argc == 3 ? argv[2] : NULL, &src);
    }
    else if ((argc == 4 || argc == 5) && !strcmp("new", argv[1]))
    {
        int width = atoi(argv[2]);
        int height = atoi(argv[3]);
        check(width  > 0 && width  < 256, "width is out of bounds.\n");
        check(height > 0 && height < 256, "height is out of bounds.\n");

        new(argc == 5 ? argv[4] : NULL, (uint8_t)width, (uint8_t)height);
    }
    else if ((argc == 7 || argc == 8) && !strcmp("resize", argv[1]))
    {
        Font resized_font = resize(argv[6], atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
        write_font(argc == 8 ? argv[7] : NULL, &resized_font);
    }
    else
        usage(strcmp("-h", argv[1]) || strcmp("--help", argv[1]));
    
    return EXIT_SUCCESS;
}
