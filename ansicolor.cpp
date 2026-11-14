/*
 * ansicolor.cpp -- a self-contained demo of ANSI colour escape sequences.
 *
 * Copyright (C) 2026 Pufeng Du <PufengDu@gmail.com>
 * Copyright (C) 2026 DeepSeek V4-Flash
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * HOW DOES COLOURED TERMINAL OUTPUT WORK?
 * ---------------------------------------
 * Terminals understand "control sequences" that are mixed into the normal
 * stream of characters.  A colour is selected with an SGR (Select Graphic
 * Rendition) sequence, which always has the shape
 *
 *     ESC [ <parameters> m
 *
 * ESC is the escape character, byte 0x1B (written "\x1B" or "\033").
 * ESC followed by '[' is called a CSI (Control Sequence Introducer), and
 * the final 'm' says "this sequence sets text rendition".  The terminal
 * does not print these bytes; it swallows them and draws the text that
 * follows accordingly.
 *
 *     "\x1B[31m"             text colour 1 of the 16-colour set (red)
 *     "\x1B[48;5;226m"       background colour 226 of the 256-colour palette
 *     "\x1B[38;2;255;0;0m"   text colour RGB(255, 0, 0), "true colour"
 *     "\x1B[0m"              reset every attribute to its default
 *
 * The layer selector is 38 for the foreground (text) and 48 for the
 * background; the "5" that follows selects the 256-colour palette and the
 * "2" selects 24-bit RGB.  Colours are sticky: they stay active until they
 * are replaced, so every coloured piece of text should be followed by
 * ansi::reset.
 *
 * Only standard C++20 is used, no external library is required.  The demo
 * works in every Unix terminal and in modern Windows terminals (Windows
 * Terminal, VS Code).  The legacy conhost.exe console needs virtual terminal
 * processing to be enabled through a Windows API call first; that part is
 * deliberately left out to keep this file portable and easy to read.
 */

#include <array>
#include <iostream>
#include <string>
#include <string_view>

namespace ansi {

//! SGR sequence that restores the default foreground and background colours.
//! Emit it after every coloured piece of text.
inline constexpr std::string_view reset = "\x1B[0m";

//! The layer a colour applies to.  The values are the SGR parameters:
//! 38 selects the foreground colour, 48 the background colour.
enum layer { fg = 38, bg = 48 };

//! One entry of the classic 16-colour palette.
struct named_color {
    std::string_view name;  //!< mnemonic accepted by ansi::color("...")
    int code;               //!< palette index, 0..15
};

//! The 16 colours of the classic ANSI palette, in palette order.
//! The "hl-" prefix means "highlight": the bright version of a colour,
//! which most terminals map to palette indexes 8..15.
inline constexpr std::array<named_color, 16> named_colors = {{
    {"black", 0}, {"red", 1}, {"green", 2}, {"yellow", 3},
    {"blue", 4}, {"magenta", 5}, {"cyan", 6}, {"white", 7},
    {"hl-black", 8}, {"hl-red", 9}, {"hl-green", 10}, {"hl-yellow", 11},
    {"hl-blue", 12}, {"hl-magenta", 13}, {"hl-cyan", 14}, {"hl-white", 15},
}};

//! One byte's worth of values.  Colour components and palette indexes are
//! reduced to 0..255 by wrapping around, just like the original "%= 256":
//! 256 -> 0, 300 -> 44 and -1 -> 255.  A bare "%" would leave negative
//! results negative, so the remainder is shifted back into range.
inline constexpr int byte_count = 256;

constexpr int wrap_byte(int value) {
    const int remainder = value % byte_count;  // -255 .. 255
    return remainder < 0 ? remainder + byte_count : remainder;
}

//! Build the escape sequence for the 256-colour palette.
//!   0..15   the classic 16 colours (their exact shades are chosen by the
//!           terminal emulator and usually come from a colour theme)
//!   16..231 a 6x6x6 RGB cube: 16 + 36*red + 6*green + blue
//!   232..255 a ramp of 24 greys from near black to near white
//! Values outside 0..255 wrap around (300 behaves like 44).  Pass ansi::bg
//! to paint the block behind the text instead of the glyphs.
inline std::string color(int index, layer l = fg) {
    return "\x1B[" + std::to_string(static_cast<int>(l)) + ";5;" +
           std::to_string(wrap_byte(index)) + "m";
}

//! Same as above, but the colour is looked up by name:
//!     ansi::color("red")                red text
//!     ansi::color("hl-blue", ansi::bg)  blue background
//! An unknown name returns an empty string, so a typo quietly falls back
//! to the terminal's default colour instead of printing garbage.
inline std::string color(std::string_view name, layer l = fg) {
    for (const named_color &entry : named_colors) {
        if (entry.name == name) {
            return color(entry.code, l);
        }
    }
    return {};
}

//! Build an escape sequence for 24-bit "true colour": red, green and blue
//! are each in 0..255, and values outside that range wrap around.  True
//! colour is supported by every modern terminal.
inline std::string rgb(int red, int green, int blue, layer l = fg) {
    return "\x1B[" + std::to_string(static_cast<int>(l)) + ";2;" +
           std::to_string(wrap_byte(red)) + ";" +
           std::to_string(wrap_byte(green)) + ";" +
           std::to_string(wrap_byte(blue)) + "m";
}

}  // namespace ansi

// ---------------------------------------------------------------------------
// Demo sections
// ---------------------------------------------------------------------------

//! Print a section heading in bright cyan so the output is easy to follow.
void print_heading(std::string_view text) {
    std::cout << '\n'
              << ansi::color("hl-cyan") << "== " << text << " =="
              << ansi::reset << '\n';
}

//! Demo 1: the 16 colours that have names.
void demo_named_colors() {
    print_heading("1. the 16 named colours (SGR 38/48;5;<index>m)");

    for (const ansi::named_color &entry : ansi::named_colors) {
        // Escape sequences are simply concatenated: the first one sets the
        // text colour, the second one paints the block behind the text.
        // A single ansi::reset undoes both of them.
        std::cout << ansi::color("hl-white")
                  << ansi::color(entry.code, ansi::bg)
                  << ' ' << entry.name << ' ' << ansi::reset;
    }
    std::cout << "\n\n"
              << ansi::color("yellow") << "Tip: " << ansi::reset
              << "a colour only affects the text printed after its escape "
                 "sequence.\n";
}

//! Demo 2: every entry of the 256-colour palette as a coloured block.
void demo_256_colors() {
    print_heading("2. the 256-colour palette (16 columns per row)");

    for (int index = 0; index < 256; ++index) {
        // Two blanks per cell make the background blocks easy to see.
        std::cout << ansi::color(index, ansi::bg) << "  ";
        if (index % 16 == 15) {
            std::cout << ansi::reset << '\n';
        }
    }
}

//! Demo 3: 24-bit RGB colour, shown as a 6x6x6 cube of blocks.
void demo_truecolor() {
    print_heading("3. 24-bit true colour (SGR 38/48;2;<r>;<g>;<b>m)");

    for (int red = 0; red < 6; ++red) {         // 6 steps per channel
        for (int green = 0; green < 6; ++green) {
            for (int blue = 0; blue < 6; ++blue) {
                std::cout << ansi::rgb(red * 51, green * 51, blue * 51, ansi::bg)
                          << "  ";
            }
            std::cout << ansi::reset << '\n';
        }
    }
}

int main() {
    demo_named_colors();
    demo_256_colors();
    demo_truecolor();

    // A last example that combines a background and a foreground colour.
    std::cout << '\n'
              << ansi::color("hl-yellow", ansi::bg) << ansi::color("black")
              << " Done! Colour is just bytes in the output stream. "
              << ansi::reset << '\n';
    return 0;
}
