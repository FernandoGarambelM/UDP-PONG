#include "utils.h"
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

/**
 * Obtiene el tiempo actual en milisegundos
 */
uint32_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

/**
 * Limita un valor a un rango específico
 */
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/**
 * Imprime un mensaje con timestamp
 */
void log_msg(const char *format, ...) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    printf("[%02d:%02d:%02d] ", t->tm_hour, t->tm_min, t->tm_sec);
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
    fflush(stdout);
}
