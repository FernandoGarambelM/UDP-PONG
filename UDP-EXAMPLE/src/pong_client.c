#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ncurses.h>
#include "protocol.h"
#include "utils.h"
#include "stats.h"

// Ventanas de ncurses
WINDOW *game_win;
WINDOW *stats_win;

// Estado del cliente
struct sockaddr_in server_addr;
int sockfd;
uint8_t my_player_id = 0;
struct server_message last_state;
struct network_stats client_stats;
uint32_t last_send_time = 0;

/**
 * Inicializa ncurses
 */
void init_ncurses(void) {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    
    // Habilitar colores si están disponibles
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLACK);  // Normal
        init_pair(2, COLOR_GREEN, COLOR_BLACK);  // Paletas
        init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Pelota
        init_pair(4, COLOR_CYAN, COLOR_BLACK);   // Estadísticas
    }
    
    // Crear ventana del juego (izquierda)
    game_win = newwin(30, 60, 1, 2);
    box(game_win, 0, 0);
    
    // Crear ventana de estadísticas (derecha)
    stats_win = newwin(30, 35, 1, 64);
    box(stats_win, 0, 0);
    
    refresh();
}

/**
 * Limpia ncurses
 */
void cleanup_ncurses(void) {
    delwin(game_win);
    delwin(stats_win);
    endwin();
}

/**
 * Renderiza el campo de juego
 */
void render_game(void) {
    werase(game_win);
    box(game_win, 0, 0);
    
    // Título
    wattron(game_win, A_BOLD);
    mvwprintw(game_win, 0, 22, " PONG GAME ");
    wattroff(game_win, A_BOLD);
    
    // Escalar coordenadas del juego a la ventana
    int win_height = 28;
    int win_width = 58;
    
    float scale_x = win_width / FIELD_WIDTH;
    float scale_y = win_height / FIELD_HEIGHT;
    
    // Dibujar línea central
    for (int y = 1; y < win_height; y++) {
        mvwaddch(game_win, y, win_width / 2, ':');
    }
    
    // Dibujar paleta jugador 1 (izquierda)
    wattron(game_win, COLOR_PAIR(2));
    int paddle1_screen_y = (int)(last_state.paddle1_y * scale_y);
    int paddle_height_screen = (int)(PADDLE_HEIGHT * scale_y);
    for (int i = 0; i < paddle_height_screen; i++) {
        int y = paddle1_screen_y - paddle_height_screen / 2 + i + 1;
        if (y >= 1 && y < win_height) {
            mvwaddch(game_win, y, 2, ACS_VLINE);
        }
    }
    wattroff(game_win, COLOR_PAIR(2));
    
    // Dibujar paleta jugador 2 (derecha)
    wattron(game_win, COLOR_PAIR(2));
    int paddle2_screen_y = (int)(last_state.paddle2_y * scale_y);
    for (int i = 0; i < paddle_height_screen; i++) {
        int y = paddle2_screen_y - paddle_height_screen / 2 + i + 1;
        if (y >= 1 && y < win_height) {
            mvwaddch(game_win, y, win_width - 1, ACS_VLINE);
        }
    }
    wattroff(game_win, COLOR_PAIR(2));
    
    // Dibujar pelota
    wattron(game_win, COLOR_PAIR(3) | A_BOLD);
    int ball_screen_x = (int)(last_state.ball_x * scale_x) + 1;
    int ball_screen_y = (int)(last_state.ball_y * scale_y) + 1;
    if (ball_screen_x >= 1 && ball_screen_x < win_width && 
        ball_screen_y >= 1 && ball_screen_y < win_height) {
        mvwaddch(game_win, ball_screen_y, ball_screen_x, 'O');
    }
    wattroff(game_win, COLOR_PAIR(3) | A_BOLD);
    
    // Mostrar puntuación
    wattron(game_win, A_BOLD);
    mvwprintw(game_win, win_height + 1, 5, "P1: %d", last_state.score1);
    mvwprintw(game_win, win_height + 1, win_width - 10, "P2: %d", last_state.score2);
    wattroff(game_win, A_BOLD);
    
    // Controles
    mvwprintw(game_win, win_height + 2, 2, "Controles: W=Arriba S=Abajo Q=Salir");
    
    wrefresh(game_win);
}

/**
 * Renderiza el panel de estadísticas
 */
void render_stats(void) {
    werase(stats_win);
    box(stats_win, 0, 0);
    
    // Título
    wattron(stats_win, A_BOLD | COLOR_PAIR(4));
    mvwprintw(stats_win, 0, 8, " ESTADISTICAS ");
    wattroff(stats_win, A_BOLD | COLOR_PAIR(4));
    
    wattron(stats_win, COLOR_PAIR(4));
    
    // Estadísticas de red
    mvwprintw(stats_win, 2, 2, "=== RED ===");
    mvwprintw(stats_win, 3, 2, "RTT:        %5.1f ms", client_stats.rtt_current);
    mvwprintw(stats_win, 4, 2, "RTT Prom:   %5.1f ms", client_stats.rtt_avg);
    mvwprintw(stats_win, 5, 2, "Perdida:    %5u %%", stats_get_loss_percent(&client_stats));
    
    mvwprintw(stats_win, 7, 2, "Enviados:   %5u", client_stats.packets_sent);
    mvwprintw(stats_win, 8, 2, "Recibidos:  %5u", client_stats.packets_received);
    mvwprintw(stats_win, 9, 2, "Perdidos:   %5u", client_stats.packets_lost);
    
    // Estadísticas del juego
    mvwprintw(stats_win, 11, 2, "=== JUEGO ===");
    mvwprintw(stats_win, 12, 2, "Jugador 1:  %5u pts", last_state.score1);
    mvwprintw(stats_win, 13, 2, "Jugador 2:  %5u pts", last_state.score2);
    
    // Información de la pelota
    mvwprintw(stats_win, 15, 2, "=== PELOTA ===");
    mvwprintw(stats_win, 16, 2, "Pos X:      %5.1f", last_state.ball_x);
    mvwprintw(stats_win, 17, 2, "Pos Y:      %5.1f", last_state.ball_y);
    
    // Estado de conexión
    mvwprintw(stats_win, 19, 2, "=== CONEXION ===");
    if (my_player_id > 0) {
        wattron(stats_win, A_BOLD);
        mvwprintw(stats_win, 20, 2, "Estado:     CONECTADO");
        mvwprintw(stats_win, 21, 2, "ID:         Jugador %d", my_player_id);
        wattroff(stats_win, A_BOLD);
    } else {
        mvwprintw(stats_win, 20, 2, "Estado:     ESPERANDO...");
    }
    
    // Información del servidor
    mvwprintw(stats_win, 23, 2, "=== SERVIDOR ===");
    mvwprintw(stats_win, 24, 2, "Timestamp:  %u", last_state.timestamp);
    
    wattroff(stats_win, COLOR_PAIR(4));
    
    wrefresh(stats_win);
}

/**
 * Envía un mensaje al servidor
 */
void send_message(struct client_message *msg) {
    msg->timestamp = get_time_ms();
    sendto(sockfd, msg, sizeof(*msg), 0, 
           (struct sockaddr *)&server_addr, sizeof(server_addr));
    stats_packet_sent(&client_stats, sizeof(*msg));
    last_send_time = msg->timestamp;
}

/**
 * Conecta al servidor
 */
int connect_to_server(const char *player_name) {
    struct client_message msg;
    memset(&msg, 0, sizeof(msg));
    
    msg.type = MSG_JOIN;
    strncpy(msg.player_name, player_name, PLAYER_NAME_LEN - 1);
    
    send_message(&msg);
    
    // Esperar respuesta del servidor
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    
    // Configurar timeout de 5 segundos
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    ssize_t received = recvfrom(sockfd, &last_state, sizeof(last_state), 0,
                                (struct sockaddr *)&from_addr, &from_len);
    
    if (received > 0 && last_state.type == MSG_STATE) {
        stats_packet_received(&client_stats, received);
        
        // Calcular RTT
        uint32_t rtt = get_time_ms() - last_send_time;
        stats_update_rtt(&client_stats, rtt);
        
        // Obtener ID asignado por el servidor
        my_player_id = last_state.player_id;
        
        // Volver a non-blocking
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
        
        return 1;
    }
    
    return 0;
}

int main(void) {
    char player_name[PLAYER_NAME_LEN];
    
    // Solicitar nombre del jugador
    printf("Ingresa tu nombre: ");
    fgets(player_name, PLAYER_NAME_LEN, stdin);
    player_name[strcspn(player_name, "\n")] = 0; // Remover newline
    
    if (strlen(player_name) == 0) {
        strcpy(player_name, "Player");
    }
    
    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error al crear socket");
        return 1;
    }
    
    // Configurar dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    // Inicializar estadísticas
    stats_init(&client_stats);
    memset(&last_state, 0, sizeof(last_state));
    
    printf("Conectando al servidor...\n");
    
    if (!connect_to_server(player_name)) {
        printf("Error: No se pudo conectar al servidor\n");
        close(sockfd);
        return 1;
    }
    
    printf("Conectado! Eres el Jugador %d\n", my_player_id);
    sleep(1);
    
    // Inicializar ncurses
    init_ncurses();
    
    int8_t current_action = ACTION_IDLE;
    int running = 1;
    uint32_t last_frame = get_time_ms();
    
    // Loop principal del cliente
    while (running) {
        uint32_t current_time = get_time_ms();
        
        // Leer input del teclado
        int ch = getch();
        int8_t new_action = ACTION_IDLE;
        
        if (ch == 'w' || ch == 'W') {
            new_action = ACTION_UP;
        } else if (ch == 's' || ch == 'S') {
            new_action = ACTION_DOWN;
        } else if (ch == 'q' || ch == 'Q') {
            running = 0;
        }
        
        // Enviar input si cambió o cada 16ms
        if (new_action != current_action || current_time - last_send_time >= FRAME_TIME_MS) {
            current_action = new_action;
            
            struct client_message msg;
            memset(&msg, 0, sizeof(msg));
            msg.type = MSG_INPUT;
            msg.player_id = my_player_id;
            msg.action = current_action;
            
            send_message(&msg);
        }
        
        // Recibir estado del servidor (non-blocking)
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        ssize_t received = recvfrom(sockfd, &last_state, sizeof(last_state), 
                                    MSG_DONTWAIT,
                                    (struct sockaddr *)&from_addr, &from_len);
        
        if (received > 0) {
            stats_packet_received(&client_stats, received);
            
            // Calcular RTT
            uint32_t rtt = current_time - last_send_time;
            stats_update_rtt(&client_stats, rtt);
        } else if (received < 0 && current_time - last_send_time > 1000) {
            // Si no recibimos nada en 1 segundo, contar como pérdida
            stats_packet_lost(&client_stats);
        }
        
        // Renderizar a 60 FPS
        if (current_time - last_frame >= FRAME_TIME_MS) {
            render_game();
            render_stats();
            last_frame = current_time;
        }
        
        usleep(1000); // 1ms
    }
    
    // Enviar mensaje de desconexión
    struct client_message msg;
    memset(&msg, 0, sizeof(msg));
    msg.type = MSG_LEAVE;
    msg.player_id = my_player_id;
    send_message(&msg);
    
    // Limpiar
    cleanup_ncurses();
    close(sockfd);
    
    printf("\n¡Gracias por jugar!\n");
    stats_print(&client_stats);
    
    return 0;
}
