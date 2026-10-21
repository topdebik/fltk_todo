// -----------------------------------------------------------------------------
// Theme.h
//
// Единая палитра приложения. Цвета собраны в одном месте, чтобы оформление
// строк, кнопок и фона нельзя было случайно разъехаться по разным файлам.
// -----------------------------------------------------------------------------

#pragma once

#include <FL/Enumerations.H>

namespace todo::theme {

inline Fl_Color background() { return fl_rgb_color(0xF3, 0xF4, 0xF8); }
inline Fl_Color card() { return fl_rgb_color(0xFF, 0xFF, 0xFF); }
inline Fl_Color doneCard() { return fl_rgb_color(0xEE, 0xF0, 0xF5); }
inline Fl_Color text() { return fl_rgb_color(0x1F, 0x29, 0x37); }
inline Fl_Color mutedText() { return fl_rgb_color(0x6B, 0x72, 0x80); }
inline Fl_Color doneText() { return fl_rgb_color(0x9C, 0xA3, 0xAF); }
inline Fl_Color accent() { return fl_rgb_color(0x4F, 0x46, 0xE5); }
inline Fl_Color accentDark() { return fl_rgb_color(0x43, 0x38, 0xCA); }
inline Fl_Color error() { return fl_rgb_color(0xB9, 0x1C, 0x1C); }

}  // namespace todo::theme
