#include "stats.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * Inicializa la estructura de estadísticas
 */
void stats_init(struct network_stats *stats) {
    memset(stats, 0, sizeof(struct network_stats));
    stats->start_time = time(NULL);
    stats->last_update = stats->start_time;
    stats->rtt_min = INFINITY;
    stats->rtt_max = 0;
    stats->rtt_avg = 0;
}

/**
 * Actualiza el RTT con una nueva medición
 */
void stats_update_rtt(struct network_stats *stats, float rtt_ms) {
    stats->rtt_current = rtt_ms;
    
    // Actualizar mínimo y máximo
    if (rtt_ms < stats->rtt_min) stats->rtt_min = rtt_ms;
    if (rtt_ms > stats->rtt_max) stats->rtt_max = rtt_ms;
    
    // Calcular promedio móvil (EWMA - Exponentially Weighted Moving Average)
    if (stats->rtt_avg == 0) {
        stats->rtt_avg = rtt_ms;
    } else {
        stats->rtt_avg = 0.9 * stats->rtt_avg + 0.1 * rtt_ms;
    }
}

/**
 * Registra un paquete enviado
 */
void stats_packet_sent(struct network_stats *stats, int bytes) {
    stats->packets_sent++;
    stats->bytes_sent += bytes;
    stats->last_update = time(NULL);
}

/**
 * Registra un paquete recibido
 */
void stats_packet_received(struct network_stats *stats, int bytes) {
    stats->packets_received++;
    stats->bytes_received += bytes;
    stats->last_update = time(NULL);
}

/**
 * Registra un paquete perdido
 */
void stats_packet_lost(struct network_stats *stats) {
    stats->packets_lost++;
}

/**
 * Calcula el porcentaje de pérdida de paquetes
 */
uint8_t stats_get_loss_percent(struct network_stats *stats) {
    if (stats->packets_sent == 0) return 0;
    
    float loss = (float)stats->packets_lost / (float)stats->packets_sent * 100.0f;
    
    if (loss > 100) loss = 100;
    if (loss < 0) loss = 0;
    
    return (uint8_t)loss;
}

/**
 * Calcula el throughput en bits por segundo
 */
void stats_calculate_throughput(struct network_stats *stats) {
    time_t now = time(NULL);
    time_t elapsed = now - stats->start_time;
    
    if (elapsed > 0) {
        stats->throughput_bps = (stats->bytes_sent * 8.0f) / elapsed;
    }
}

/**
 * Imprime las estadísticas en consola
 */
void stats_print(struct network_stats *stats) {
    stats_calculate_throughput(stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║     ESTADÍSTICAS DE RED UDP            ║\n");
    printf("╠════════════════════════════════════════╣\n");
    printf("║ Paquetes Enviados:    %-16u ║\n", stats->packets_sent);
    printf("║ Paquetes Recibidos:   %-16u ║\n", stats->packets_received);
    printf("║ Paquetes Perdidos:    %-16u ║\n", stats->packets_lost);
    printf("║ Pérdida:              %-15u%% ║\n", stats_get_loss_percent(stats));
    printf("║                                        ║\n");
    printf("║ RTT Actual:           %-13.1f ms ║\n", stats->rtt_current);
    printf("║ RTT Promedio:         %-13.1f ms ║\n", stats->rtt_avg);
    printf("║ RTT Mínimo:           %-13.1f ms ║\n", stats->rtt_min);
    printf("║ RTT Máximo:           %-13.1f ms ║\n", stats->rtt_max);
    printf("║                                        ║\n");
    printf("║ Throughput:           %-13.1f bps ║\n", stats->throughput_bps);
    printf("║ Bytes Enviados:       %-16llu ║\n", (unsigned long long)stats->bytes_sent);
    printf("║ Bytes Recibidos:      %-16llu ║\n", (unsigned long long)stats->bytes_received);
    printf("╚════════════════════════════════════════╝\n\n");
}
