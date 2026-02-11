#pragma once
#include "Include.h"

enum LogType {
    Error,
    Warning,
    Info,
    Success
};

class Logger {
public:
    void CreateConsole( ) {
        if (AllocConsole( )) {
            freopen_s( (FILE**)stdout, "CONOUT$", "w", stdout );
        }
    }

    template<typename... Args>
    void Log( LogType level, Args&&... args ) {
        std::lock_guard<std::mutex> lock( mutex_ );

        std::ostringstream oss;
        (oss << ... << args);

        std::cout
            << "[" << GetColor( level ) << ToString( level ) << "\033[0m" << "] "
            << "[" << GetTime( ) << "] "
            << oss.str( )
            << std::endl;
    }

private:
    std::string GetTime( ) {
        auto t = std::time( nullptr );
        std::tm tm;
#ifdef _WIN32
        localtime_s( &tm, &t );
#else
        localtime_r( &t, &tm );
#endif
        std::ostringstream oss;
        oss << std::put_time( &tm, "%H:%M" );
        return oss.str( );
    }

    const char* GetColor( LogType level ) {
        switch (level) {
        case Error:   return "\033[31m"; // Red
        case Warning: return "\033[33m"; // Yellow
        case Info:    return "\033[36m"; // Cyan
        case Success: return "\033[32m"; // Green
        default:      return "\033[0m";
        }
    }

    const char* ToString( LogType level ) {
        switch (level) {
        case Error:   return "ERROR";
        case Warning: return "WARNING";
        case Info:    return "INFO";
        case Success: return "SUCCESS";
        default:      return "LOG";
        }
    }

    std::mutex mutex_;
};

inline std::unique_ptr<Logger> g_Logger = std::make_unique<Logger>( );