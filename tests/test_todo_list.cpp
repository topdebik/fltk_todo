// -----------------------------------------------------------------------------
// tests/test_todo_list.cpp
//
// Тесты модели. Специально не используем сторонний фреймворк: студенту полезно
// увидеть, что проверка поведения — это обычные вызовы функций и сравнения.
//
// Программа возвращает 0, если все проверки прошли, и 1 при любой ошибке.
// -----------------------------------------------------------------------------

#include "TodoList.h"

#include <exception>
#include <iostream>
#include <string>

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

template <typename ExpectedException, typename Function>
void expectThrows(Function&& function, const std::string& description) {
    try {
        function();
    } catch (const ExpectedException&) {
        std::cout << "[ OK ] " << description << '\n';
        return;
    } catch (const std::exception& error) {
        ++failures;
        std::cout << "[FAIL] " << description << " (другое исключение: "
                  << error.what() << ")\n";
        return;
    } catch (...) {
        ++failures;
        std::cout << "[FAIL] " << description << " (неизвестное исключение)\n";
        return;
    }

    ++failures;
    std::cout << "[FAIL] " << description << " (исключение не выброшено)\n";
}

}  // namespace

int main() {
    using todo::TodoList;

    std::cout << "Проверка TodoList\n";
    std::cout << "-----------------\n";

    TodoList list;

    expect(list.size() == 0, "новый список пуст");
    expect(list.doneCount() == 0, "в новом списке нет выполненных дел");

    const todo::ItemId milk = list.add("  Купить молоко  ");
    expect(list.size() == 1, "после добавления в списке одно дело");
    expect(list.items().front().title == "Купить молоко",
           "пробелы по краям названия удалены");
    expect(list.items().front().id == milk, "add() вернул id нового дела");
    expect(!list.items().front().done, "новое дело не выполнено");

    expectThrows<std::invalid_argument>(
        [&list] { list.add("   \t\n  "); },
        "пустое название отклоняется с std::invalid_argument");

    const todo::ItemId homework = list.add("Сделать домашку");
    expect(homework != milk, "идентификаторы дел различаются");

    list.markDone(milk);
    expect(list.doneCount() == 1, "отметка дела увеличивает счётчик выполненного");
    expect(list.items().front().done, "флаг done установлен у нужного дела");
    expect(!list.items().back().done, "второе дело осталось невыполненным");

    list.markDone(milk);
    expect(list.doneCount() == 1, "повторная отметка идемпотентна");

    list.remove(milk);
    expect(list.size() == 1, "удаление уменьшает размер списка");
    expect(list.items().front().id == homework,
           "после удаления остаётся нужное дело");

    expectThrows<std::out_of_range>(
        [&list, milk] { list.markDone(milk); },
        "нельзя отметить удалённое дело");
    expectThrows<std::out_of_range>(
        [&list, milk] { list.remove(milk); },
        "нельзя удалить дело повторно");

    const todo::ItemId newTask = list.add("Новое дело");
    expect(newTask > homework, "id удалённого дела не переиспользуется");

    list.remove(newTask);
    list.remove(homework);
    expect(list.size() == 0, "список можно очистить удалением всех дел");
    expect(list.doneCount() == 0, "счётчик выполненного корректен после очистки");

    if (failures == 0) {
        std::cout << "\nВсе проверки модели пройдены.\n";
        return 0;
    }

    std::cout << "\nПровалено проверок: " << failures << '\n';
    return 1;
}
