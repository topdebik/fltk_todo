# -----------------------------------------------------------------------------
# CMakeLists.txt — сборка учебного TODO-листа.
#
# Требования:
#   * компилятор с поддержкой C++17;
#   * FLTK 1.3 или новее с заголовками разработки (libfltk1.3-dev в Ubuntu).
#
# Типичные команды:
#   cmake -S . -B build
#   cmake --build build
#   ctest --test-dir build --output-on-failure
# -----------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.16)

project(fltk_todo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Включает BUILD_TESTING (по умолчанию ON) и команду ctest.
include(CTest)

# find_package ищет стандартный модуль FindFLTK.cmake, который, в свою очередь,
# использует установленный в системе fltk-config. Именно этот пакет приходит
# из apt вместе с libfltk1.3-dev.
find_package(FLTK REQUIRED)

# Единые предупреждения для всех наших целей. Это не ошибки, но полезный
# «второй взгляд» компилятора на код.
function(todo_enable_warnings target)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# --- Модель ------------------------------------------------------------------
# Модель не зависит от FLTK: её можно собрать и протестировать без экрана.
add_library(todo_model STATIC
    src/TodoList.cpp
)
target_include_directories(todo_model PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
todo_enable_warnings(todo_model)

# --- Интерфейс ---------------------------------------------------------------
# Отдельная библиотека нужна, чтобы и приложение, и UI-тесты использовали один
# и тот же код без повторной компиляции.
add_library(todo_ui STATIC
    src/TodoRow.cpp
    src/TodoWindow.cpp
)
target_include_directories(todo_ui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FLTK_INCLUDE_DIRS}
)
target_link_libraries(todo_ui PUBLIC
    todo_model
    ${FLTK_LIBRARIES}
)
todo_enable_warnings(todo_ui)

# --- Приложение --------------------------------------------------------------
add_executable(todo src/main.cpp)
target_link_libraries(todo PRIVATE todo_ui)
todo_enable_warnings(todo)

# --- Тесты -------------------------------------------------------------------
if(BUILD_TESTING)
    # Тест модели работает без графического окружения.
    add_executable(todo_model_tests tests/test_todo_list.cpp)
    target_link_libraries(todo_model_tests PRIVATE todo_model)
    todo_enable_warnings(todo_model_tests)
    add_test(NAME todo_model COMMAND todo_model_tests)

    # UI-тест нажимает на настоящие виджеты FLTK, поэтому ему нужен дисплей.
    # Если DISPLAY не задан, тест сам вернёт код 77 — CTest пометит его как
    # пропущенный, а не как упавший.
    add_executable(todo_ui_tests tests/test_todo_ui.cpp)
    target_link_libraries(todo_ui_tests PRIVATE todo_ui)
    todo_enable_warnings(todo_ui_tests)
    add_test(NAME todo_ui COMMAND todo_ui_tests)
    set_tests_properties(todo_ui PROPERTIES SKIP_RETURN_CODE 77)
endif()
