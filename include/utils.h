#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

/**
 * Obtiene el tiempo actual en milisegundos
 * @return Timestamp en milisegundos
 */
uint32_t get_time_ms(void);

/**
 * Limita un valor a un rango específico
 * @param value Valor a limitar
 * @param min Valor mínimo
 * @param max Valor máximo
 * @return Valor limitado entre min y max
 */
float clamp(float value, float min, float max);

/**
 * Imprime un mensaje con timestamp
 * @param format Formato del mensaje (estilo printf)
 */
void log_msg(const char *format, ...);

#endif // UTILS_H
