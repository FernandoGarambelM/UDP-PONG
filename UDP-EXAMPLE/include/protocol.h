#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// Configuración del protocolo
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define MAX_PLAYERS 2
#define PLAYER_NAME_LEN 16

// Tipos de mensajes: Cliente -> Servidor
#define MSG_JOIN 1
#define MSG_INPUT 2
#define MSG_STATS 3
#define MSG_LEAVE 4

// Tipos de mensajes: Servidor -> Cliente
#define MSG_STATE 1
#define MSG_STATS_RESPONSE 2
#define MSG_ERROR 3

// Acciones del jugador
#define ACTION_DOWN -1
#define ACTION_IDLE 0
#define ACTION_UP 1

// Dimensiones del juego (unidades lógicas)
#define FIELD_WIDTH 100.0f
#define FIELD_HEIGHT 100.0f
#define PADDLE_WIDTH 2.0f
#define PADDLE_HEIGHT 15.0f
#define BALL_SIZE 2.0f

// Velocidades
#define PADDLE_SPEED 4.5f  // Aumentada para mejor control
#define BALL_SPEED 0.6f    // Reducida para mejor jugabilidad

// FPS del juego
#define TARGET_FPS 60
#define FRAME_TIME_MS (1000 / TARGET_FPS)

/**
 * Mensaje del Cliente al Servidor
 * Tamaño: 26 bytes
 */
struct client_message {
    uint8_t type;              // Tipo de mensaje (JOIN, INPUT, STATS, LEAVE)
    uint32_t timestamp;        // Timestamp en milisegundos
    uint8_t player_id;         // ID del jugador (0 si es JOIN)
    int8_t action;             // -1=ABAJO, 0=QUIETO, 1=ARRIBA
    char player_name[PLAYER_NAME_LEN];  // Nombre del jugador (solo para JOIN)
} __attribute__((packed));

/**
 * Mensaje del Servidor al Cliente
 * Tamaño: 40 bytes
 */
struct server_message {
    uint8_t type;              // Tipo de mensaje (STATE, STATS, ERROR)
    uint32_t timestamp;        // Timestamp del servidor
    uint8_t player_id;         // ID asignado al jugador (solo en respuesta a JOIN)
    
    // Estado del juego
    float paddle1_y;           // Posición Y paleta jugador 1 (0-100)
    float paddle2_y;           // Posición Y paleta jugador 2 (0-100)
    float ball_x;              // Posición X pelota (0-100)
    float ball_y;              // Posición Y pelota (0-100)
    uint8_t score1;            // Puntos jugador 1
    uint8_t score2;            // Puntos jugador 2
    
    // Estadísticas de red
    uint16_t rtt_ms;           // Round Trip Time en ms
    uint8_t loss_percent;      // Porcentaje de pérdida (0-100)
    uint32_t packets_sent;     // Total paquetes enviados
    uint32_t packets_recv;     // Total paquetes recibidos
} __attribute__((packed));

#endif // PROTOCOL_H
