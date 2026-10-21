// -----------------------------------------------------------------------------
// TodoList.cpp
//
// Реализация модели. Здесь нет ни одного вызова FLTK: этот файл можно собрать
// и протестировать как обычную консольную программу.
// -----------------------------------------------------------------------------

#include "TodoList.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace todo {

namespace {

/// Удаляет пробельные символы с начала и конца строки.
///
/// Работаем с ASCII-пробелами: для учебного примера этого достаточно. Важно,
/// что функция возвращает новую строку и не меняет аргумент — так проще
/// рассуждать о владении данными.
std::string trim(const std::string& text) {
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return {};
    }

    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

}  // namespace

ItemId TodoList::add(std::string title) {
    std::string cleaned = trim(title);
    if (cleaned.empty()) {
        throw std::invalid_argument("Название дела не может быть пустым.");
    }

    // Сначала запоминаем id, но увеличиваем счётчик только после успешной
    // вставки. Если push_back выбросит std::bad_alloc, состояние останется
    // согласованным.
    const ItemId id = nextId_;
    items_.push_back(TodoItem{id, std::move(cleaned), false});
    ++nextId_;
    return id;
}

void TodoList::markDone(ItemId id) {
    findById(id).done = true;
}

void TodoList::remove(ItemId id) {
    const auto it = std::find_if(items_.begin(), items_.end(),
                                 [id](const TodoItem& item) { return item.id == id; });

    if (it == items_.end()) {
        throw std::out_of_range("Дело с указанным id не найдено.");
    }

    items_.erase(it);
}

std::size_t TodoList::doneCount() const noexcept {
    return static_cast<std::size_t>(
        std::count_if(items_.begin(), items_.end(),
                      [](const TodoItem& item) { return item.done; }));
}

TodoItem& TodoList::findById(ItemId id) {
    const auto it = std::find_if(items_.begin(), items_.end(),
                                 [id](const TodoItem& item) { return item.id == id; });

    if (it == items_.end()) {
        throw std::out_of_range("Дело с указанным id не найдено.");
    }

    return *it;
}

}  // namespace todo
