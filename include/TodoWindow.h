// -----------------------------------------------------------------------------
// TodoWindow.h
//
// Главное окно приложения. Оно выполняет роль контроллера:
//   * создаёт виджеты FLTK;
//   * принимает события от кнопки, поля ввода и строк;
//   * вызывает методы модели TodoList;
//   * перестраивает список после каждого изменения.
//
// Модель и интерфейс разделены: TodoWindow знает про TodoList, а TodoList
// ничего не знает про окно. Это упрощает тестирование и объяснение кода.
// -----------------------------------------------------------------------------

#pragma once

#include "TodoList.h"

#include <FL/Fl_Window.H>

#include <cstddef>
#include <string>
#include <vector>

class Fl_Box;
class Fl_Button;
class Fl_Input;
class Fl_Scroll;
class Fl_Widget;

namespace todo {

/// Главное (и единственное) окно TODO-листа.
///
/// Окно имеет фиксированный размер: так проще для первой практики, потому что
/// не нужно писать отдельный код перерасчёта координат при изменении размера.
class TodoWindow : public Fl_Window {
public:
    TodoWindow();

    /// Добавляет дело по тексту. Пустой текст приводит к сообщению об ошибке
    /// в интерфейсе, а не к аварийному завершению программы.
    void addTask(const std::string& title);

    /// Реакция на клик по галочке.
    /// @param id      дело, с которым взаимодействует пользователь;
    /// @param checked true — галочку поставили, false — сняли.
    void toggleTask(ItemId id, bool checked);

    /// Количество дел (удобно для автотестов).
    std::size_t taskCount() const noexcept { return model_.size(); }

    /// Количество выполненных дел (удобно для автотестов).
    std::size_t doneCount() const noexcept { return model_.doneCount(); }

private:
    /// Статический callback кнопки «Добавить» и клавиши Enter в поле ввода.
    static void addClicked(Fl_Widget* widget, void* userData);

    /// Создаёт все статические виджеты окна.
    void buildUi();

    /// Удаляет старые строки и создаёт новые по текущему состоянию модели.
    /// @param scrollToBottom прокрутить список вниз (после добавления дела).
    void rebuildList(bool scrollToBottom);

    /// Обновляет подпись «Выполнено N из M».
    void updateCounter();

    /// Показывает сообщение под заголовком.
    void showMessage(const std::string& text, bool isError);

    TodoList model_;

    Fl_Input* input_ = nullptr;
    Fl_Button* addButton_ = nullptr;
    Fl_Scroll* scroll_ = nullptr;
    Fl_Box* message_ = nullptr;
    Fl_Box* counter_ = nullptr;
    Fl_Box* hint_ = nullptr;

    /// Строки и заглушка пустого списка, которыми владеет scroll_.
    /// Вектор нужен только для того, чтобы аккуратно удалять старые виджеты
    /// при перестройке списка.
    std::vector<Fl_Widget*> dynamicWidgets_;
};

}  // namespace todo
