// -----------------------------------------------------------------------------
// TodoWindow.cpp
//
// Здесь собирается весь интерфейс. Основная идея файла:
//   событие FLTK -> маленький метод-действие -> изменение модели -> перерисовка.
//
// Перерисовка сделана простым и надёжным способом: старые строки удаляются,
// новые создаются по данным модели. Для маленького TODO-листа это быстрее
// писать и проще читать, чем точечно обновлять отдельные виджеты.
// -----------------------------------------------------------------------------

#include "TodoWindow.h"
#include "Theme.h"
#include "TodoRow.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Scroll.H>

#include <algorithm>
#include <exception>
#include <string>

namespace todo {

namespace {

constexpr int kWindowWidth = 560;
constexpr int kWindowHeight = 600;
constexpr int kMargin = 20;
constexpr int kRowHeight = 48;
constexpr int kRowGap = 6;
constexpr int kScrollWidth = kWindowWidth - 2 * kMargin;

/// Ширина вертикальной полосы прокрутки и небольшой зазор до неё.
constexpr int kScrollbarReserve = 18;

}  // namespace

TodoWindow::TodoWindow()
    : Fl_Window(kWindowWidth, kWindowHeight, "TODO-лист — практика на FLTK") {
    color(theme::background());

    // Фиксируем размеры: вся вёрстка рассчитана на 560x600.
    // Для учебного примера это лучше, чем забыть пересчитать координаты
    // при растягивании окна и получить «съехавший» интерфейс.
    size_range(kWindowWidth, kWindowHeight, kWindowWidth, kWindowHeight);

    begin();
    buildUi();
    end();

    rebuildList(false);
    updateCounter();
}

void TodoWindow::buildUi() {
    auto* title = new Fl_Box(kMargin, 16, kScrollWidth, 34, "Мои дела");
    title->box(FL_NO_BOX);
    title->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    title->labelfont(FL_HELVETICA_BOLD);
    title->labelsize(28);
    title->labelcolor(theme::text());

    auto* subtitle =
        new Fl_Box(kMargin, 52, kScrollWidth, 18, "Практика: C++17 · FLTK · CMake");
    subtitle->box(FL_NO_BOX);
    subtitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    subtitle->labelsize(12);
    subtitle->labelcolor(theme::mutedText());

    message_ = new Fl_Box(kMargin, 80, kScrollWidth, 18,
                          "Введите дело и нажмите «Добавить» или Enter.");
    message_->box(FL_NO_BOX);
    message_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    message_->labelsize(12);
    message_->labelcolor(theme::mutedText());

    input_ = new Fl_Input(kMargin, 106, 404, 40);
    input_->box(FL_DOWN_BOX);
    input_->color(FL_WHITE);
    input_->textcolor(theme::text());
    input_->textfont(FL_HELVETICA);
    input_->textsize(15);
    input_->when(FL_WHEN_ENTER_KEY);
    input_->callback(&TodoWindow::addClicked, this);
    input_->tooltip("Введите дело и нажмите Enter");

    addButton_ = new Fl_Button(436, 106, 104, 40, "Добавить");
    addButton_->box(FL_ROUND_UP_BOX);
    addButton_->color(theme::accent());
    addButton_->selection_color(theme::accentDark());
    addButton_->labelcolor(FL_WHITE);
    addButton_->labelfont(FL_HELVETICA_BOLD);
    addButton_->labelsize(14);
    addButton_->callback(&TodoWindow::addClicked, this);
    addButton_->tooltip("Добавить дело (Enter в поле ввода)");

    // Область списка. Строки будут прямыми детьми Fl_Scroll — так FLTK сам
    // прокручивает их и обрезает по границам области.
    scroll_ = new Fl_Scroll(kMargin, 158, kScrollWidth, 384);
    scroll_->type(Fl_Scroll::VERTICAL);
    scroll_->box(FL_NO_BOX);
    scroll_->color(theme::background());
    scroll_->begin();
    scroll_->end();

    counter_ = new Fl_Box(kMargin, 554, 260, 18, "");
    counter_->box(FL_NO_BOX);
    counter_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    counter_->labelsize(12);
    counter_->labelcolor(theme::mutedText());

    hint_ = new Fl_Box(300, 554, 240, 18,
                       "Повторный клик по галочке удаляет дело");
    hint_->box(FL_NO_BOX);
    hint_->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    hint_->labelsize(11);
    hint_->labelcolor(theme::mutedText());

    input_->take_focus();
}

void TodoWindow::addClicked(Fl_Widget* /*widget*/, void* userData) {
    auto* window = static_cast<TodoWindow*>(userData);
    window->addTask(window->input_->value());
}

void TodoWindow::addTask(const std::string& title) {
    try {
        model_.add(title);

        // Очищаем поле только после успешного добавления: если пользователь
        // ошибся, введённый текст не потеряется.
        input_->value("");
        rebuildList(true);
        updateCounter();
        showMessage("Дело добавлено. Отметьте галочку, когда выполните.", false);
    } catch (const std::exception& error) {
        showMessage(error.what(), true);
    }

    input_->take_focus();
}

void TodoWindow::toggleTask(ItemId id, bool checked) {
    try {
        if (checked) {
            model_.markDone(id);
            showMessage("Готово! Нажмите на галочку ещё раз, чтобы удалить дело.",
                        false);
        } else {
            model_.remove(id);
            showMessage("Выполненное дело удалено.", false);
        }
    } catch (const std::exception& error) {
        // Сюда можно попасть, если состояние интерфейса и модели разошлось.
        // Вместо падения показываем понятное сообщение и перерисовываем список.
        showMessage(error.what(), true);
    }

    rebuildList(false);
    updateCounter();
}

void TodoWindow::rebuildList(bool scrollToBottom) {
    const int previousScroll = scroll_->yposition();

    // Удаляем старые строки. Fl::delete_widget() откладывает удаление до конца
    // обработки текущего события: это важно, потому что перестройка может
    // происходить прямо из callback самой галочки. Обычный delete уничтожил бы
    // виджет, который ещё находится на стеке обработки события FLTK.
    for (Fl_Widget* widget : dynamicWidgets_) {
        scroll_->remove(widget);
        Fl::delete_widget(widget);
    }
    dynamicWidgets_.clear();

    // Сбрасываем прокрутку, чтобы новые строки создавались от верхней границы.
    scroll_->scroll_to(0, 0);

    const int rowWidth = scroll_->w() - kScrollbarReserve;
    int contentHeight = 0;
    scroll_->begin();

    if (model_.items().empty()) {
        auto* empty = new Fl_Box(scroll_->x(), scroll_->y() + 20, rowWidth, 48,
                                 "Пока дел нет. Добавьте первое!");
        empty->box(FL_NO_BOX);
        empty->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        empty->labelfont(FL_HELVETICA_ITALIC);
        empty->labelsize(13);
        empty->labelcolor(theme::mutedText());
        dynamicWidgets_.push_back(empty);
    } else {
        int y = scroll_->y();

        for (const TodoItem& item : model_.items()) {
            auto* row = new TodoRow(item, scroll_->x(), y, rowWidth, kRowHeight);

            // Захватываем this окна. Это безопасно: строки являются детьми
            // окна и не могут пережить его.
            row->onToggle([this](ItemId id, bool checked) {
                toggleTask(id, checked);
            });

            dynamicWidgets_.push_back(row);
            y += kRowHeight + kRowGap;
        }

        // Высота всего содержимого. Последний отступ снизу можно не учитывать:
        // он нужен только для визуального воздуха.
        contentHeight = y - scroll_->y();
    }

    scroll_->end();
    scroll_->redraw();

    // Максимальная прокрутка — это высота содержимого минус высота окна.
    // Важно посчитать её самим: Fl_Scroll::scroll_to() не ограничивает
    // аргумент и сдвигает детей ровно на переданное значение.
    const int maxScroll = std::max(0, contentHeight - scroll_->h());
    const int target = scrollToBottom ? maxScroll
                                      : std::min(previousScroll, maxScroll);
    scroll_->scroll_to(0, target);
}

void TodoWindow::updateCounter() {
    std::string text;

    if (model_.size() == 0) {
        text = "Дел пока нет";
    } else {
        text = "Выполнено " + std::to_string(model_.doneCount()) + " из " +
               std::to_string(model_.size());
    }

    // copy_label() важен: text — локальная строка, а label() сохраняет
    // только указатель на её буфер.
    counter_->copy_label(text.c_str());
    counter_->redraw();
}

void TodoWindow::showMessage(const std::string& text, bool isError) {
    // Используем copy_label(), а не label(): label() запоминает указатель,
    // и временная строка стала бы висячим указателем.
    message_->copy_label(text.c_str());
    message_->labelcolor(isError ? theme::error() : theme::mutedText());
    message_->redraw();
}

}  // namespace todo
