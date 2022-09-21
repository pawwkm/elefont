Elefont - ASCII font packer
===========================

> Sir? Sir? America's on the other end of that sidewalk. You're on international soil. For your sake, I hope you brought the keys for that.

> I have no-no gripes with you at all, okay? I've been calling my congressman. I've been writing United Nations.

> I don't care.

> Did you know that elephants eat over 500 pounds of food per day?! And their dung-It can be.. It can be processed into parchment.

> You're about to be in a big heap of that, sir.

> ELAPHANTS!

> (In his walkie) Front gate, now.

> ELAPHANTS!

For my [text editor](https://github.com/pawwkm/moss) I have spent hours on extracting characters from fonts I were interested in trying out for it. Pixel by pixel... by hand. So I created this tool. This tool converts the [ASCII](https://en.wikipedia.org/wiki/ASCII) portion of [BDF](https://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format) fonts to a text or binary format. It can also embed the binary format in C code.
Fonts are monospaced, monochrome and ASCII only. Fonts encode `'!'` to  `'~'` in that order. It is assumed that space is a blank character. Info such as offsets are not stored.

# Formats
Both the binary and the text formats (.af and .txt respectively) starts off with the width and height of the characters followed by a bit array of pixels.
Characters are stored from `'!'` to  `'~'` and the pixels are stored form the top left to the bottom right. Here is a sample of a 2x3 font bitmap:

```
!!!!!!"" """"#### ##$$$$$$ ''''''((
(((()))) ))****** and so on. 
```

## Text (.txt)
For finer edits or creating a new font. The text format allows for piggy backing on existing editors for undo/redo and whatnot.
Text based fonts starts with the width and height separated by a single space and ends with a newline. After that there must
be `width * height * 94` `#` and `-` optionally separated by spaces and newlines.

```
2 3

--
--
--

--
--
--

...
```
## Binary (.af)
Binare fonts start with 2 bytes which are the width and height followed by the bit array of pixels.

## C code (.c)
Useful for [embedding](https://github.com/pawwkm/moss/blob/main/configuration.c#L43-L49) fonts directly into programs.

```c
#define CHARACTERS_IN_FONT 94

typedef struct
{
    uint8_t* bitmap;
    uint8_t width;
    uint8_t height;
} Font;

const Font font = 
{
    .bitmap = (uint8_t[917]) { /* ... */ },
    .width = 6,
    .height = 13
};
```

# Example
In my case [Scientifica](https://github.com/nerdypepper/scientifica) and [Creep](https://github.com/romeovs/creep) needed no modification after this command:

```console
elefont creep.bdf
```

```
7 12

-------
-------
--#----
--#----
--#----
--#----
--#----
-------
--#----
-------
-------
-------

-------
-------
-#-#---
-#-#---
-#-#---
-------
-------
-------
-------
-------
-------
-------

...
```

Since I didn't specify `dst` the font was displayed in the console.

I needed to adjust [Cozette](https://github.com/slavfox/Cozette) a bit. While the ASCII part fits 6x13px it defines a lot more than ASCII and needs 12x13px to fit all of it. Take `H` for example. 

```
------------
------------
-#---#------
-#---#------
-#---#------
-#####------
-#---#------
-#---#------
-#---#------
-#---#------
------------
------------
------------
```

It would take awhile to shave off the excess to get all characters to down to 6x13px. But still faster than the zooming in on picture of the font to make out each pixel at a time... 
Anyway use the `resize` command to adjust the columns and rows across all characters:

```console
elefont resize 0 0 -1 -5 cozette.bdf
```

```
------
------
#---#-
#---#-
#---#-
#####-
#---#-
#---#-
#---#-
#---#-
------
------
------
```

## --help
```
elefont src [dst] | command
     src    Path to the file to convert or print as the .txt format if dst is omitted.
            .bdf, .txt and .af files are supported.
     [dst]  The path to store the font at. .txt, .af and .c files are supported.

     commands
         new       Creates a new font.
           width   The width of the font in pixels. From 1 to 255.
           height  The height of the font in pixels. From 1 to 255.
           [dst]   The path to store the font at.

         resize    Add and/or subtract rows and columns of pixels.
           t       Prepends t rows to the top.
           b       Appends b rows to the bottom.
           l       Prepends l columns to the left.
           r       Appends r columns to the right.
           src     The path to the font to resize.
           [dst]   The path to store the font at.

         -h --help Displays this message.
```

# Building
I only have a build script for Windows but Elefont only depends on libc so it should not be much work to add more if needed.

## Windows
1. Open up the [Visual Studio Developer Command Prompt](https://docs.microsoft.com/en-us/visualstudio/ide/reference/command-prompt-powershell?view=vs-2019) with the `x64` environment.
2. Navigate to the root directory.
3. Run `build.bat`.`

`elefont.exe` sits in the root directory.
