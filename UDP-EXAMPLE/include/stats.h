#ifndef STATS_H
#define STATS_H

#include <stdint.h>
#include <time.h>

/**
 * Estructura para almacenar estadísticas de red
 */
struct network_stats {
    // Contadores de paquetes
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t packets_lost;
    
    // Bytes transferidos
    uint64_t bytes_sent;
    uint64_t bytes_received;
    
    // Latencia (RTT - Round Trip Time)
    float rtt_min;
    float rtt_max;
    float rtt_avg;
    float rtt_current;
    
    // Rendimiento
    float throughput_bps;
    
    // Timestamps
    time_t start_time;
    time_t last_update;
};

/**
 * Inicializa la estructura de estadísticas
 */
void stats_init(struct network_stats *stats);

/**
 * Actualiza el RTT con una nueva medición
 */
void stats_update_rtt(struct network_stats *stats, float rtt_ms);

/**
 * Registra un paquete enviado
 */
void stats_packet_sent(struct network_stats *stats, int bytes);

/**
 * Registra un paquete recibido
 */
void stats_packet_received(struct network_stats *stats, int bytes);

/**
 * Registra un paquete perdido
 */
void stats_packet_lost(struct network_stats *stats);

/**
 * Calcula el porcentaje de pérdida de paquetes
 */
uint8_t stats_get_loss_percent(struct network_stats *stats);

/**
 * Calcula el throughput en bits por segundo
 */
void stats_calculate_throughput(struct network_stats *stats);

/**
 * Imprime las estadísticas en consola
 */
void stats_print(struct network_stats *stats);

#endif // STATS_H
