// -----------------------------------------------------------------------------
// main.cpp
//
// Точка входа. Здесь намеренно нет логики приложения: только создание окна,
// запуск цикла событий FLTK и обработка исключений, которые могут возникнуть
// при создании интерфейса.
// -----------------------------------------------------------------------------

#include "TodoWindow.h"

#include <FL/Fl.H>

#include <exception>
#include <iostream>
#include <memory>

int main(int argc, char** argv) {
    try {
        // std::unique_ptr — пример RAII: окно автоматически уничтожится при
        // выходе из main, даже если Fl::run() завершится с ошибкой.
        auto window = std::make_unique<todo::TodoWindow>();

        window->show(argc, argv);

        // Fl::run() возвращает управление, когда все окна закрыты.
        return Fl::run();
    } catch (const std::exception& error) {
        std::cerr << "Не удалось запустить приложение: " << error.what() << '\n';
        return 1;
    }
}
