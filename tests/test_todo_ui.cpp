// -----------------------------------------------------------------------------
// tests/test_todo_ui.cpp
//
// Интеграционный тест интерфейса. Он создаёт настоящее окно FLTK и отправляет
// в него события мыши — те же самые, что приходят от пользователя.
//
// Тесту нужен графический дисплей (DISPLAY). Если его нет, программа ничего не
// проверяет и возвращает 77. В CMake этот код помечен как SKIP_RETURN_CODE,
// поэтому ctest покажет «Skipped», а не «Failed».
// -----------------------------------------------------------------------------

#include "TodoWindow.h"

#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Window.H>

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void expect(bool condition, const std::string& description) {
    if (condition) {
        std::cout << "[ OK ] " << description << '\n';
    } else {
        ++failures;
        std::cout << "[FAIL] " << description << '\n';
    }
}

/// Рекурсивно ищет первое поле ввода в дереве виджетов.
Fl_Input* findInput(Fl_Widget* root) {
    if (auto* input = dynamic_cast<Fl_Input*>(root)) {
        return input;
    }

    if (auto* group = dynamic_cast<Fl_Group*>(root)) {
        for (int i = 0; i < group->children(); ++i) {
            if (Fl_Input* found = findInput(group->child(i))) {
                return found;
            }
        }
    }

    return nullptr;
}

/// Ищет кнопку по точной надписи.
Fl_Button* findButton(Fl_Widget* root, const std::string& label) {
    if (auto* button = dynamic_cast<Fl_Button*>(root)) {
        if (button->label() != nullptr && label == button->label()) {
            return button;
        }
    }

    if (auto* group = dynamic_cast<Fl_Group*>(root)) {
        for (int i = 0; i < group->children(); ++i) {
            if (Fl_Button* found = findButton(group->child(i), label)) {
                return found;
            }
        }
    }

    return nullptr;
}

void collectCheckBoxes(Fl_Widget* root, std::vector<Fl_Check_Button*>& result) {
    if (auto* checkBox = dynamic_cast<Fl_Check_Button*>(root)) {
        result.push_back(checkBox);
        return;
    }

    if (auto* group = dynamic_cast<Fl_Group*>(root)) {
        for (int i = 0; i < group->children(); ++i) {
            collectCheckBoxes(group->child(i), result);
        }
    }
}

std::vector<Fl_Check_Button*> checkBoxes(Fl_Widget* root) {
    std::vector<Fl_Check_Button*> result;
    collectCheckBoxes(root, result);
    return result;
}

/// Имитирует одиночный клик левой кнопкой мыши по виджету.
void click(Fl_Window& window, Fl_Widget* widget) {
    Fl::e_x = widget->x() + widget->w() / 2;
    Fl::e_y = widget->y() + widget->h() / 2;
    Fl::e_x_root = window.x() + Fl::e_x;
    Fl::e_y_root = window.y() + Fl::e_y;
    Fl::e_keysym = FL_Button + 1;  // Fl::event_button() == 1
    Fl::e_state = FL_BUTTON1;
    Fl::e_clicks = 1;
    Fl::e_is_click = 1;

    Fl::handle(FL_PUSH, &window);
    Fl::handle(FL_RELEASE, &window);

    // Даём FLTK доработать отложенные удаления виджетов.
    Fl::check();
}

}  // namespace

int main() {
    const char* display = std::getenv("DISPLAY");
    if (display == nullptr || *display == '\0') {
        std::cout << "UI-тест пропущен: переменная DISPLAY не задана.\n";
        return 77;
    }

    std::cout << "Проверка интерфейса TodoWindow\n";
    std::cout << "------------------------------\n";

    todo::TodoWindow window;
    window.show();
    Fl::check();

    Fl_Input* input = findInput(&window);
    Fl_Button* addButton = findButton(&window, "Добавить");

    expect(input != nullptr, "поле ввода найдено");
    expect(addButton != nullptr, "кнопка «Добавить» найдена");
    if (input == nullptr || addButton == nullptr) {
        return 1;
    }

    // 1. Пустое дело не должно добавляться.
    input->value("   ");
    click(window, addButton);
    expect(window.taskCount() == 0, "пустое дело не добавляется");
    expect(checkBoxes(&window).empty(), "для пустого дела строка не создана");

    // 2. Добавление через кнопку и очистка поля ввода.
    input->value("  Купить хлеб  ");
    click(window, addButton);
    expect(window.taskCount() == 1, "кнопка добавляет дело");
    expect(window.doneCount() == 0, "новое дело не выполнено");
    expect(std::string(input->value()).empty(), "поле ввода очищено");

    std::vector<Fl_Check_Button*> boxes = checkBoxes(&window);
    expect(boxes.size() == 1, "в списке появилась одна строка");
    expect(!boxes.empty() && boxes.front()->value() == 0,
           "галочка нового дела снята");

    // 3. Добавление по Enter (имитируем callback поля ввода).
    input->value("Позвонить маме");
    input->do_callback();
    Fl::check();
    expect(window.taskCount() == 2, "Enter в поле ввода добавляет дело");

    // 4. Первый клик по галочке отмечает дело выполненным.
    boxes = checkBoxes(&window);
    expect(boxes.size() == 2, "в списке две строки");
    if (boxes.size() == 2) {
        click(window, boxes.front());
        expect(window.taskCount() == 2, "отметка не удаляет дело");
        expect(window.doneCount() == 1, "дело отмечено выполненным");

        boxes = checkBoxes(&window);
        expect(boxes.size() == 2 && boxes.front()->value() == 1,
               "галочка отрисована в состоянии «выполнено»");
    }

    // 5. Повторный клик по установленной галочке удаляет дело.
    if (!boxes.empty()) {
        click(window, boxes.front());
        expect(window.taskCount() == 1, "повторный клик удаляет выполненное дело");
        expect(window.doneCount() == 0, "выполненных дел не осталось");

        boxes = checkBoxes(&window);
        expect(boxes.size() == 1, "в списке осталась одна строка");
        expect(!boxes.empty() && boxes.front()->value() == 0,
               "оставшееся дело не выполнено");
    }

    // 6. Длинный список должен прокручиваться, а не уезжать за границы окна.
    for (int i = 0; i < 10; ++i) {
        window.addTask("Дело " + std::to_string(i + 1));
    }
    Fl::check();

    boxes = checkBoxes(&window);
    expect(window.taskCount() == 11, "в длинном списке 11 дел");
    expect(boxes.size() == 11, "для каждого дела создана строка");
    expect(!boxes.empty() && boxes.front()->y() < 158,
           "верхние строки прокручены за пределы видимой области");
    expect(!boxes.empty() && boxes.back()->y() + boxes.back()->h() <= 542,
           "последняя строка видна после прокрутки вниз");

    window.hide();
    Fl::check();

    if (failures == 0) {
        std::cout << "\nВсе проверки интерфейса пройдены.\n";
        return 0;
    }

    std::cout << "\nПровалено проверок: " << failures << '\n';
    return 1;
}
