# Общие настройки для всех проектов
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Устанавливаем глобальные пути для выходных файлов
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

# Функция для настройки выходных путей
function(set_output_directories target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
        ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    )
endfunction()

# Функция для добавления определений Unicode
function(add_unicode_definitions target)
    target_compile_definitions(${target} PRIVATE 
        UNICODE
        _UNICODE
    )
endfunction()