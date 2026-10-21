// -----------------------------------------------------------------------------
// TodoRow.h
//
// TodoRow — одна строка списка: квадратная галочка слева и текст дела справа.
//
// Строка не хранит копию всего списка и не удаляет дела сама. Она только
// сообщает наружу: «по моей галочке щёлкнули, вот мой id и новое состояние».
// Решение принимает TodoWindow. Такой подход называется «виджет-представление
// + callback» и позволяет легко переиспользовать строку.
// -----------------------------------------------------------------------------

#pragma once

#include "TodoItem.h"

#include <FL/Fl_Group.H>

#include <functional>

class Fl_Box;
class Fl_Check_Button;

namespace todo {

/// Виджет одной строки TODO-листа.
///
/// Жизненным циклом строки владеет Fl_Scroll: он является её родителем.
/// TodoRow не удаляет себя сам — это делает окно через Fl::delete_widget(),
/// когда перестраивает список.
class TodoRow : public Fl_Group {
public:
    /// Обработчик нажатия на галочку.
    /// @param id       идентификатор дела;
    /// @param checked  true, если галочка теперь стоит.
    using ToggleHandler = std::function<void(ItemId id, bool checked)>;

    /// Создаёт строку по данным дела.
    TodoRow(const TodoItem& item, int x, int y, int width, int height);

    /// Сохраняет обработчик нажатия. Обычно вызывается сразу после создания.
    void onToggle(ToggleHandler handler);

    /// Доступ к галочке — нужен, например, автотестам.
    Fl_Check_Button* checkButton() const noexcept { return check_; }

private:
    /// Статический callback FLTK: у C-совместимых callback нет доступа к this,
    /// поэтому указатель на объект передаётся через user_data.
    static void checkboxClicked(Fl_Widget* widget, void* userData);

    ItemId id_;
    Fl_Check_Button* check_ = nullptr;
    Fl_Box* title_ = nullptr;
    ToggleHandler toggleHandler_;
};

}  // namespace todo
