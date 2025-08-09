#pragma once
#include <framework/stdafx.h>
#include <memory>

class Console
{
private:
    HANDLE m_consoleHandle;
    FILE* m_consoleStream;

public:
    // Constructor create console
    Console();
    
    // Destructor automaticly handle console
    ~Console();
    
    // Disallow copy
    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;
    
    // Allow move   
    Console(Console&& other) noexcept;
    Console& operator=(Console&& other) noexcept;
    
    void CreateConsole();
    void CleanupConsole();
    
    bool IsValid() const { return m_consoleHandle != nullptr; }
    
    static std::unique_ptr<Console> Create();
};

struct ConsoleDeleter {
    void operator()(Console* console) {
        if (console) {
            delete console;
        }
    }
};

using ConsolePtr = std::unique_ptr<Console, ConsoleDeleter>;
