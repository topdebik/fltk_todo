// -----------------------------------------------------------------------------
// TodoRow.cpp
//
// Собираем строку из двух стандартных виджетов FLTK:
//   Fl_Check_Button — квадратная галочка;
//   Fl_Box          — текст дела.
//
// Никакой ручной отрисовки здесь нет: стандартные виджеты проще понять
// новичку и они автоматически работают с клавиатурой и наведением мыши.
// -----------------------------------------------------------------------------

#include "TodoRow.h"
#include "Theme.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>

#include <string>
#include <utility>

namespace todo {

namespace {

/// Экранирует символ '@' для надписей FLTK.
///
/// FLTK считает '@' началом спецсимвола (например, "@->" рисует стрелку).
/// Пользователь может ввести такое сочетание в название дела, поэтому
/// удваиваем '@': по документации FLTK "@@" выводит один символ '@'.
std::string escapeLabel(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size() + 4);

    for (const char symbol : text) {
        if (symbol == '@') {
            escaped.push_back('@');
        }
        escaped.push_back(symbol);
    }

    return escaped;
}

}  // namespace

TodoRow::TodoRow(const TodoItem& item, int x, int y, int width, int height)
    : Fl_Group(x, y, width, height), id_(item.id) {
    // Цвет фона строки зависит от состояния дела. FL_FLAT_BOX рисует
    // однотонный прямоугольник без рамки.
    box(FL_FLAT_BOX);
    color(item.done ? theme::doneCard() : theme::card());

    begin();

    constexpr int kCheckboxSize = 26;
    constexpr int kCheckboxMargin = 14;

    check_ = new Fl_Check_Button(x + kCheckboxMargin,
                                 y + (height - kCheckboxSize) / 2,
                                 kCheckboxSize,
                                 kCheckboxSize);
    check_->box(FL_NO_BOX);          // без фона вокруг квадратика
    check_->down_box(FL_DOWN_BOX);   // сам квадратик галочки
    // У Fl_Check_Button размер квадратика равен labelsize(), поэтому
    // увеличиваем его отдельно от размера текста строки.
    check_->labelsize(20);
    check_->color(FL_WHITE);
    check_->selection_color(theme::accent());
    check_->value(item.done ? 1 : 0);
    check_->tooltip(item.done ? "Удалить выполненное дело"
                              : "Отметить как выполненное");
    check_->callback(&TodoRow::checkboxClicked, this);

    const std::string label = escapeLabel(item.title);

    // label() только запоминает указатель, а label — локальная строка,
    // которая умрёт в конце конструктора. Поэтому используем copy_label()
    // и copy_tooltip(): FLTK сам выделит и освободит память под текст.
    title_ = new Fl_Box(x + 52, y, width - 66, height, "");
    title_->copy_label(label.c_str());
    title_->box(FL_NO_BOX);
    title_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CLIP);
    title_->labelsize(14);
    title_->copy_tooltip(label.c_str());

    if (item.done) {
        title_->labelcolor(theme::doneText());
        title_->labelfont(FL_HELVETICA_ITALIC);
    } else {
        title_->labelcolor(theme::text());
        title_->labelfont(FL_HELVETICA);
    }

    end();
}

void TodoRow::onToggle(ToggleHandler handler) {
    toggleHandler_ = std::move(handler);
}

void TodoRow::checkboxClicked(Fl_Widget* widget, void* userData) {
    // static_cast безопасен: callback всегда получает именно наш чекбокс и
    // указатель на TodoRow, который мы передали при создании.
    auto* checkbox = static_cast<Fl_Check_Button*>(widget);
    auto* row = static_cast<TodoRow*>(userData);

    if (row->toggleHandler_) {
        // FLTK уже изменил value() кнопки до вызова callback:
        //   0 -> пользователь снял галочку (в нашем UX это удаление);
        //   1 -> пользователь поставил галочку.
        row->toggleHandler_(row->id_, checkbox->value() != 0);
    }
}

}  // namespace todo
