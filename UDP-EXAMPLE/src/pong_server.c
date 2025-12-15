#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <math.h>
#include "protocol.h"
#include "utils.h"
#include "stats.h"

// Estructura para información del jugador
struct player_info {
    struct sockaddr_in addr;
    socklen_t addr_len;
    uint8_t id;
    char name[PLAYER_NAME_LEN];
    time_t last_seen;
    int active;
    int8_t last_action;
};

// Estructura para el estado del juego
struct game_state {
    float paddle1_y;
    float paddle2_y;
    float ball_x;
    float ball_y;
    float ball_vx;
    float ball_vy;
    uint8_t score1;
    uint8_t score2;
};

// Variables globales
struct player_info players[MAX_PLAYERS];
struct game_state game;
struct network_stats stats;
int num_players = 0;

/**
 * Inicializa el estado del juego
 */
void init_game(void) {
    game.paddle1_y = FIELD_HEIGHT / 2.0f;
    game.paddle2_y = FIELD_HEIGHT / 2.0f;
    game.ball_x = FIELD_WIDTH / 2.0f;
    game.ball_y = FIELD_HEIGHT / 2.0f;
    game.ball_vx = BALL_SPEED;
    game.ball_vy = BALL_SPEED * 0.5f;
    game.score1 = 0;
    game.score2 = 0;
    
    memset(players, 0, sizeof(players));
    stats_init(&stats);
}

/**
 * Reinicia la pelota al centro
 */
void reset_ball(void) {
    game.ball_x = FIELD_WIDTH / 2.0f;
    game.ball_y = FIELD_HEIGHT / 2.0f;
    
    // Velocidad aleatoria
    game.ball_vx = (rand() % 2 == 0 ? 1 : -1) * BALL_SPEED;
    game.ball_vy = ((rand() % 100) / 100.0f - 0.5f) * BALL_SPEED;
}

/**
 * Registra un nuevo jugador
 */
int register_player(struct sockaddr_in *addr, socklen_t addr_len, const char *name) {
    if (num_players >= MAX_PLAYERS) {
        return -1; // Servidor lleno
    }
    
    int player_idx = num_players;
    players[player_idx].addr = *addr;
    players[player_idx].addr_len = addr_len;
    players[player_idx].id = player_idx + 1;
    strncpy(players[player_idx].name, name, PLAYER_NAME_LEN - 1);
    players[player_idx].last_seen = time(NULL);
    players[player_idx].active = 1;
    players[player_idx].last_action = ACTION_IDLE;
    
    num_players++;
    
    log_msg("🎮 Jugador %d conectado: %s", players[player_idx].id, name);
    
    return players[player_idx].id;
}

/**
 * Actualiza la física del juego
 */
void update_physics(void) {
    // Actualizar posiciones de paletas según acciones
    if (num_players > 0 && players[0].active) {
        game.paddle1_y += players[0].last_action * PADDLE_SPEED;
        game.paddle1_y = clamp(game.paddle1_y, PADDLE_HEIGHT / 2, 
                               FIELD_HEIGHT - PADDLE_HEIGHT / 2);
    }
    
    if (num_players > 1 && players[1].active) {
        game.paddle2_y += players[1].last_action * PADDLE_SPEED;
        game.paddle2_y = clamp(game.paddle2_y, PADDLE_HEIGHT / 2, 
                               FIELD_HEIGHT - PADDLE_HEIGHT / 2);
    }
    
    // Actualizar posición de la pelota
    game.ball_x += game.ball_vx;
    game.ball_y += game.ball_vy;
    
    // Rebote en paredes superior e inferior
    if (game.ball_y <= BALL_SIZE / 2 || game.ball_y >= FIELD_HEIGHT - BALL_SIZE / 2) {
        game.ball_vy = -game.ball_vy;
        game.ball_y = clamp(game.ball_y, BALL_SIZE / 2, FIELD_HEIGHT - BALL_SIZE / 2);
    }
    
    // Colisión con paleta izquierda (jugador 1)
    if (game.ball_x <= PADDLE_WIDTH + BALL_SIZE / 2) {
        if (fabs(game.ball_y - game.paddle1_y) <= PADDLE_HEIGHT / 2) {
            game.ball_vx = fabs(game.ball_vx); // Rebote hacia la derecha
            game.ball_x = PADDLE_WIDTH + BALL_SIZE / 2;
            
            // Agregar efecto según dónde golpea
            float hit_pos = (game.ball_y - game.paddle1_y) / (PADDLE_HEIGHT / 2);
            game.ball_vy += hit_pos * 0.5f;
        }
    }
    
    // Colisión con paleta derecha (jugador 2)
    if (game.ball_x >= FIELD_WIDTH - PADDLE_WIDTH - BALL_SIZE / 2) {
        if (fabs(game.ball_y - game.paddle2_y) <= PADDLE_HEIGHT / 2) {
            game.ball_vx = -fabs(game.ball_vx); // Rebote hacia la izquierda
            game.ball_x = FIELD_WIDTH - PADDLE_WIDTH - BALL_SIZE / 2;
            
            // Agregar efecto según dónde golpea
            float hit_pos = (game.ball_y - game.paddle2_y) / (PADDLE_HEIGHT / 2);
            game.ball_vy += hit_pos * 0.5f;
        }
    }
    
    // Gol del jugador 2 (pelota sale por la izquierda)
    if (game.ball_x < 0) {
        game.score2++;
        log_msg("⚽ GOL! Jugador 2 anota. Marcador: %d - %d", game.score1, game.score2);
        reset_ball();
    }
    
    // Gol del jugador 1 (pelota sale por la derecha)
    if (game.ball_x > FIELD_WIDTH) {
        game.score1++;
        log_msg("⚽ GOL! Jugador 1 anota. Marcador: %d - %d", game.score1, game.score2);
        reset_ball();
    }
}

/**
 * Procesa un mensaje del cliente
 */
void process_client_message(int sockfd, struct client_message *msg, 
                            struct sockaddr_in *client_addr, socklen_t addr_len) {
    
    if (msg->type == MSG_JOIN) {
        // Registrar nuevo jugador
        int player_id = register_player(client_addr, addr_len, msg->player_name);
        
        // Enviar confirmación (estado actual)
        struct server_message response;
        memset(&response, 0, sizeof(response));
        response.type = MSG_STATE;
        response.timestamp = get_time_ms();
        response.player_id = player_id;  // Enviar ID asignado
        response.paddle1_y = game.paddle1_y;
        response.paddle2_y = game.paddle2_y;
        response.ball_x = game.ball_x;
        response.ball_y = game.ball_y;
        response.score1 = game.score1;
        response.score2 = game.score2;
        
        if (player_id > 0) {
            sendto(sockfd, &response, sizeof(response), 0, 
                   (struct sockaddr *)client_addr, addr_len);
            stats_packet_sent(&stats, sizeof(response));
        }
        
    } else if (msg->type == MSG_INPUT) {
        // Actualizar acción del jugador
        int player_idx = msg->player_id - 1;
        if (player_idx >= 0 && player_idx < num_players) {
            players[player_idx].last_action = msg->action;
            players[player_idx].last_seen = time(NULL);
        }
        
    } else if (msg->type == MSG_LEAVE) {
        // Desconectar jugador
        int player_idx = msg->player_id - 1;
        if (player_idx >= 0 && player_idx < num_players) {
            players[player_idx].active = 0;
            log_msg("👋 Jugador %d desconectado: %s", 
                   players[player_idx].id, players[player_idx].name);
        }
    }
}

/**
 * Envía el estado del juego a todos los jugadores activos
 */
void broadcast_state(int sockfd) {
    struct server_message state;
    memset(&state, 0, sizeof(state));
    
    state.type = MSG_STATE;
    state.timestamp = get_time_ms();
    state.paddle1_y = game.paddle1_y;
    state.paddle2_y = game.paddle2_y;
    state.ball_x = game.ball_x;
    state.ball_y = game.ball_y;
    state.score1 = game.score1;
    state.score2 = game.score2;
    
    // Estadísticas
    state.rtt_ms = (uint16_t)stats.rtt_avg;
    state.loss_percent = stats_get_loss_percent(&stats);
    state.packets_sent = stats.packets_sent;
    state.packets_recv = stats.packets_received;
    
    // Enviar a todos los jugadores activos
    for (int i = 0; i < num_players; i++) {
        if (players[i].active) {
            sendto(sockfd, &state, sizeof(state), 0, 
                   (struct sockaddr *)&players[i].addr, players[i].addr_len);
            stats_packet_sent(&stats, sizeof(state));
        }
    }
}

int main(void) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct client_message msg;
    
    // Inicializar juego
    srand(time(NULL));
    init_game();
    
    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error al crear socket");
        return 1;
    }
    
    // Configurar socket como non-blocking
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Bind
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error en bind");
        close(sockfd);
        return 1;
    }
    
    log_msg("🟢 Servidor UDP-PONG activo en puerto %d", SERVER_PORT);
    log_msg("⏳ Esperando jugadores...");
    
    uint32_t last_frame = get_time_ms();
    
    // Loop principal del servidor
    while (1) {
        uint32_t current_time = get_time_ms();
        
        // Recibir mensajes de clientes (non-blocking)
        ssize_t received = recvfrom(sockfd, &msg, sizeof(msg), 0, 
                                    (struct sockaddr *)&client_addr, &client_len);
        
        if (received > 0) {
            stats_packet_received(&stats, received);
            process_client_message(sockfd, &msg, &client_addr, client_len);
        }
        
        // Actualizar física a 60 FPS
        if (current_time - last_frame >= FRAME_TIME_MS) {
            if (num_players == MAX_PLAYERS) {
                update_physics();
                broadcast_state(sockfd);
            }
            last_frame = current_time;
        }
        
        // Pequeña pausa para no consumir 100% CPU
        usleep(1000); // 1ms
    }
    
    close(sockfd);
    return 0;
}
