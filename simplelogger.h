#pragma once
#include <Windows.h>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <string>

using std::string, std::cout, std::endl;

inline std::fstream fl;

namespace SimpleLogger {

#define Logi SimpleLogger::Logger::Log(SimpleLogger::LogLevel::LOGLEVEL_INFO)
#define Logd SimpleLogger::Logger::Log(SimpleLogger::LogLevel::LOGLEVEL_DBG)
#define Logw SimpleLogger::Logger::Log(SimpleLogger::LogLevel::LOGLEVEL_WARN)
#define Loge SimpleLogger::Logger::Log(SimpleLogger::LogLevel::LOGLEVEL_ERR)


    enum LogMode {
        LOGMODE_CONSOLE = 1 << 0,
        LOGMODE_FILE = 1 << 1,
        LOGMODE_GUI = 1 << 2
    };
    inline LogMode operator|(LogMode a, LogMode b) {
        return a = static_cast<LogMode>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline LogMode operator|=(LogMode& a, LogMode b) {
        return a = static_cast<LogMode>(static_cast<int>(a) | static_cast<int>(b));
    }
    inline LogMode operator&=(LogMode& a, LogMode b) {
        return a = static_cast<LogMode>(static_cast<int>(a) & static_cast<int>(b));
    }

    enum LogLevel {
        LOGLEVEL_INFO = 1 << 0,
        LOGLEVEL_DBG = 1 << 1,
        LOGLEVEL_WARN = 1 << 2,
        LOGLEVEL_ERR = 1 << 3
    };

    class Logger {
    public:
        ~Logger() {
            _Log("\n==End of log==");
            if(fl.is_open()) {
                fl << fflush;
                fl.close();
            }
        }

        struct LogE {
            LogE(Logger& logger) : m_logger(logger) {}
            ~LogE() {
                if(m_logger.m_mode & (LogMode::LOGMODE_CONSOLE)) {
                    auto consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
                    SetConsoleTextAttribute(consoleHandle, m_logger.m_currentColor);
                }
            }
            Logger& m_logger;
            template <typename T>
            LogE& operator<<(T const& obj) {
                m_logger._Log(obj);
                return (*this);
            }
        };

        static void Init(LogMode mode = LogMode::LOGMODE_CONSOLE, string prefix = "") {
            Instance().m_prefix = prefix;
            if((mode & (LogMode::LOGMODE_CONSOLE)) && GetConsoleWindow() == nullptr) {
                AllocConsole();
            }

            freopen_s(&Instance().m_consoleDummy, "CONIN$", "r", stdin);
            freopen_s(&Instance().m_consoleDummy, "CONOUT$", "w", stderr);
            freopen_s(&Instance().m_consoleDummy, "CONOUT$", "w", stdout);
            Instance().m_mode = mode;

            if(mode & (LogMode::LOGMODE_FILE)) {
                CreateDirectory(L"logs", NULL);
                fl.open(LogName(), std::ios::in | std::ios::out | std::ios::trunc);
                Instance()._Log("==Start of log==");
            }
        }

        static Logger& Log(LogLevel logLevel = LogLevel::LOGLEVEL_INFO) {
            auto& i = Instance();
            i.m_lastLogLevel = logLevel;
            i._Log(logLevel);
            return i;
        }

        static Logger& Instance() {
            static Logger instance;
            return instance;
        }

        template <typename T>
        LogE operator<<(T const& obj) {
            this->_Log(obj);
            return LogE(*this);
        }

        static string GenerateTimeStamp() {
            SYSTEMTIME systemTime;
            GetLocalTime(&systemTime);
            std::ostringstream stream;
            stream << std::setfill('0')
                << "["
                << systemTime.wYear << "/"
                << std::setw(2) << systemTime.wMonth << "/"
                << std::setw(2) << systemTime.wDay << " - "
                << std::setw(2) << systemTime.wHour << ":"
                << std::setw(2) << systemTime.wMinute << ":"
                << std::setw(2) << systemTime.wSecond << "."
                << std::setw(3) << systemTime.wMilliseconds << "]";
            return stream.str(); // [1970/01/01 - 00:00:00.000]
        }

        static string LogName() {
            SYSTEMTIME systemTime;
            GetLocalTime(&systemTime);
            std::ostringstream stream;
            stream << "logs/D" << std::setfill('0')
                << systemTime.wYear << "-"
                << std::setw(2) << systemTime.wMonth << "-"
                << std::setw(2) << systemTime.wDay << "_T"
                << std::setw(2) << systemTime.wHour << "-"
                << std::setw(2) << systemTime.wMinute << "-"
                << std::setw(2) << systemTime.wSecond << ".log";
            return stream.str(); // D1970-01-01_T00:00:00.log
        }

    private:
        LogMode m_mode;
        FILE* m_consoleDummy;
        string m_prefix;
        LogLevel m_lastLogLevel;
        short m_currentColor;
        void _Log(LogLevel logLevel) {
            auto ts = GenerateTimeStamp();
            auto l = _ResolveLevel(logLevel);
            if(m_mode & (LogMode::LOGMODE_FILE)) {
                fl << std::endl << ts << m_prefix << l << " ";
            }
            if(m_mode & (LogMode::LOGMODE_CONSOLE)) {
                std::cout << std::endl << ts << m_prefix << l << " ";
            }
        }

        string _ResolveLevel(LogLevel logLevel) {
            if(!GetColor(m_currentColor)) m_currentColor = 7;
            auto consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
            switch(logLevel) {
                case LOGLEVEL_INFO:
                    SetConsoleTextAttribute(consoleHandle, 3);
                    return "[INF]";
                case LOGLEVEL_DBG:
                    SetConsoleTextAttribute(consoleHandle, 8);
                    return "[DBG]";
                case LOGLEVEL_WARN:
                    SetConsoleTextAttribute(consoleHandle, 6);
                    return "[WRN]";
                case LOGLEVEL_ERR:
                    SetConsoleTextAttribute(consoleHandle, 4);
                    return "[ERR]";
            }
            return "[???]";
        }
        bool GetColor(short& ret) {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if(!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
                return false;
            ret = info.wAttributes;
            return true;
        }


        template <typename T>
        void _Log(T const& obj) {
            if(m_mode & (LogMode::LOGMODE_FILE)) {
                fl << obj << " ";
            }
            if(m_mode & (LogMode::LOGMODE_CONSOLE)) {
                std::cout << obj << " ";
            }
        }
    };
}
