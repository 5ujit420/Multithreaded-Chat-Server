#include "../include/logger.h"
#include <stdarg.h>
#include <stdio.h>

void log_message(log_level_t level, const char *fmt, ...) {
  const char *level_str;

  switch (level) {
  case LOG_INFO:
    level_str = "INFO";
    break;
  case LOG_WARN:
    level_str = "WARN";
    break;
  case LOG_ERROR:
    level_str = "ERROR";
    break;
  case LOG_DEBUG:
    level_str = "DEBUG";
    break;
  default:
    level_str = "LOG";
    break;
  }

  time_t now = time(NULL);
  char ts[20];

  strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

  fprintf(stdout, "[%s] [%s] ", ts, level_str);

  va_list args;
  va_start(args, fmt);
  vfprintf(stdout, fmt, args);
  va_end(args);

  fprintf(stdout, "\n");
  fflush(stdout);
}
