#pragma once

#include "_logger.hpp"

namespace logger {
inline void enable_file() noexcept { Logger::get_instance().enable_file(); }

inline void set_level(Logger::Level level) { Logger::get_instance().set_level(level); }

template <typename... Args>
inline void critical(string_view fmt, Args &&...args) noexcept {
  Logger::get_instance().critical(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void error(string_view fmt, Args &&...args) noexcept {
  Logger::get_instance().error(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void warn(string_view fmt, Args &&...args) noexcept {
  Logger::get_instance().warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void info(string_view fmt, Args &&...args) noexcept {
  Logger::get_instance().info(fmt, std::forward<Args>(args)...);
}

template <typename... Args>
inline void debug(string_view fmt, Args &&...args) noexcept {
  Logger::get_instance().debug(fmt, std::forward<Args>(args)...);
}
}  // namespace logger
