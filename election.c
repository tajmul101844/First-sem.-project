#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    #define IS_WINDOWS 1
    #define IS_MAC 0
#elif __APPLE__
    #include <unistd.h>
    #include <termios.h>
    #define IS_WINDOWS 0
    #define IS_MAC 1
#else
    #include <unistd.h>
    #include <termios.h>
    #define IS_WINDOWS 0
    #define IS_MAC 0
#endif

#define MAX_P 5
#define MAX_V 500
#define MAX_C 15
#define MAX_S 30
#define NID_SIZE 13
#define MAX_LOGIN_ATTEMPTS 3
#define LOCKOUT_SECONDS 60

#define RST   "\033[0m"
#define RED   "\033[1;31m"
#define GRN   "\033[1;32m"
#define YLW   "\033[1;33m"
#define BLU   "\033[1;34m"
#define MAG   "\033[1;35m"
#define CYN   "\033[1;36m"
#define WHT   "\033[1;37m"
#define BRED  "\033[1;91m"
#define BGRN  "\033[1;92m"
#define BYLW  "\033[1;93m"
#define BBLU  "\033[1;94m"
#define BMAG  "\033[1;95m"
#define BCYN  "\033[1;96m"
#define DIM   "\033[2m"
#define BOLD  "\033[1m"
#define ITAL  "\033[3m"
#define UNDR  "\033[4m"

#define SND_STARTUP    "sounds/startup.mp3"
#define SND_CLICK      "sounds/click.mp3"
#define SND_SUCCESS    "sounds/success.mp3"
#define SND_ERROR      "sounds/error.mp3"
#define SND_VOTE       "sounds/vote.mp3"
#define SND_FANFARE    "sounds/fanfare.mp3"
#define SND_BELL       "sounds/bell.mp3"
#define SND_INTRO      "sounds/intro.mp3"
#define SND_TYPING     "sounds/typing.mp3"
#define SND_WHOOSH     "sounds/whoosh.mp3"
#define SND_POP        "sounds/pop.mp3"
#define SND_DING       "sounds/ding.mp3"
#define SND_DRUM       "sounds/drum.mp3"
#define SND_COIN       "sounds/coin.mp3"
#define SND_TADA       "sounds/tada.mp3"
#define SND_LOCK       "sounds/lock.mp3"
#define SND_UNLOCK     "sounds/unlock.mp3"
#define SND_ALERT      "sounds/alert.mp3"

struct Party {
    int id;
    char name[50];
    int symbol;
    int votes[MAX_S];
    int total;
    int active;
    char leader[50];
    char slogan[100];
};

struct Voter {
    char nid[14];
    char name[50];
    int age;
    int area_id;
    int center_id;
    int voted;
    char date[15];
    char gender[10];
    char phone[15];
};

struct Area {
    int id;
    char name[50];
    int centers[MAX_S];
    int center_count;
    int voter_count;
};

struct Center {
    int id;
    char name[50];
    int area_id;
    int vote_count;
    int is_open;
    int capacity;
};

struct Election {
    int is_running;
    int party_count;
    int voter_count;
    int area_count;
    int center_count;
    char name[50];
    char date[15];
    char start_time[10];
    char end_time[10];
};

struct AdminSecurity {
    int failed_attempts;
    time_t lockout_time;
    int is_locked;
    char last_attempt_ip[20];
};

struct AuditLog {
    char timestamp[25];
    char action[100];
    char user[50];
    int log_count;
};

struct Party party[MAX_P];
struct Voter voter[MAX_V];
struct Area area[MAX_C];
struct Center center[MAX_S];
struct Election election;
struct AdminSecurity admin_security;
struct AuditLog audit_log[100];
int audit_count = 0;

char PASSWORD[30] = "admin123";
int sound_enabled = 1;
int animation_enabled = 1;
int theme_mode = 1;

char getch_custom() {
    #ifdef _WIN32
        return _getch();
    #else
        char c = 0;
        struct termios old_settings, new_settings;
        tcgetattr(0, &old_settings);
        new_settings = old_settings;
        new_settings.c_lflag &= ~ICANON;
        new_settings.c_lflag &= ~ECHO;
        tcsetattr(0, TCSANOW, &new_settings);
        c = getchar();
        tcsetattr(0, TCSADRAIN, &old_settings);
        return c;
    #endif
}

void clear_screen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void delay_ms(int ms) {
    if (!animation_enabled) ms = ms / 4;
    #ifdef _WIN32
        Sleep(ms);
    #else
        usleep(ms * 1000);
    #endif
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
}

void play_sound(const char *filename) {
    if (sound_enabled == 0) return;
    if (file_exists(filename) == 0) return;
    char command[512];
    #ifdef _WIN32
        sprintf(command, "start /min powershell -c \"(New-Object Media.SoundPlayer '%s').PlaySync()\" >nul 2>&1", filename);
    #elif __APPLE__
        sprintf(command, "afplay \"%s\" 2>/dev/null &", filename);
    #else
        sprintf(command, "mpg123 -q \"%s\" 2>/dev/null &", filename);
    #endif
    system(command);
}

void play_sound_wait(const char *filename) {
    if (sound_enabled == 0) return;
    if (file_exists(filename) == 0) return;
    char command[512];
    #ifdef _WIN32
        sprintf(command, "powershell -c \"(New-Object Media.SoundPlayer '%s').PlaySync()\" >nul 2>&1", filename);
    #elif __APPLE__
        sprintf(command, "afplay \"%s\" 2>/dev/null", filename);
    #else
        sprintf(command, "mpg123 -q \"%s\" 2>/dev/null", filename);
    #endif
    system(command);
}

void stop_all_sounds() {
    #ifdef _WIN32
        system("taskkill /F /IM wmplayer.exe >nul 2>&1");
    #elif __APPLE__
        system("killall afplay 2>/dev/null");
    #else
        system("killall mpg123 2>/dev/null");
    #endif
}

void sound_click()   { play_sound(SND_CLICK); }
void sound_success() { play_sound(SND_SUCCESS); }
void sound_error()   { play_sound(SND_ERROR); }
void sound_vote()    { play_sound(SND_VOTE); }
void sound_winner()  { play_sound_wait(SND_FANFARE); }
void sound_bell()    { play_sound(SND_BELL); }
void sound_intro()   { play_sound(SND_INTRO); }
void sound_typing()  { play_sound(SND_TYPING); }
void sound_whoosh()  { play_sound(SND_WHOOSH); }
void sound_pop()     { play_sound(SND_POP); }
void sound_ding()    { play_sound(SND_DING); }
void sound_drum()    { play_sound(SND_DRUM); }
void sound_coin()    { play_sound(SND_COIN); }
void sound_tada()    { play_sound(SND_TADA); }

void add_audit_log(const char *action, const char *user) {
    if (audit_count >= 100) audit_count = 0;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    sprintf(audit_log[audit_count].timestamp, "%04d-%02d-%02d %02d:%02d:%02d",
        tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday,
        tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
    strncpy(audit_log[audit_count].action, action, 99);
    audit_log[audit_count].action[99] = '\0';
    strncpy(audit_log[audit_count].user, user, 49);
    audit_log[audit_count].user[49] = '\0';
    audit_count++;
}

void wait_key() {
    printf("\n");
    printf("   %s╔══════════════════════════════════════╗%s\n", CYN, RST);
    printf("   %s║%s    Press any key to continue...      %s║%s\n", CYN, RST, CYN, RST);
    printf("   %s╚══════════════════════════════════════╝%s\n", CYN, RST);
    fflush(stdout);
    getch_custom();
}

int get_string(char *prompt, char *buffer, int size) {
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, size, stdin) == NULL) {
        buffer[0] = '\0';
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strlen(buffer) == 0) return 0;
    return 1;
}

int get_int(char *prompt, int *result, int min, int max) {
    char buffer[20];
    printf("%s", prompt);
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return 0;
    buffer[strcspn(buffer, "\n")] = '\0';
    if (strlen(buffer) == 0) return 0;
    char *endptr;
    long num = strtol(buffer, &endptr, 10);
    if (*endptr != '\0') return 0;
    if (num < min || num > max) return 0;
    *result = (int)num;
    return 1;
}

void animate_box_top(int width, const char *color) {
    int i;
    printf("   %s╔", color);
    fflush(stdout);
    if (animation_enabled) delay_ms(5);
    for (i = 0; i < width - 2; i++) {
        printf("═");
        fflush(stdout);
        if (animation_enabled && i % 3 == 0) delay_ms(2);
    }
    printf("╗%s\n", RST);
}

void animate_box_bottom(int width, const char *color) {
    int i;
    printf("   %s╚", color);
    fflush(stdout);
    if (animation_enabled) delay_ms(5);
    for (i = 0; i < width - 2; i++) {
        printf("═");
        fflush(stdout);
        if (animation_enabled && i % 3 == 0) delay_ms(2);
    }
    printf("╝%s\n", RST);
}

void draw_box_top(int width, const char *color) {
    int i;
    printf("   %s╔", color);
    for (i = 0; i < width - 2; i++) printf("═");
    printf("╗%s\n", RST);
}

void draw_box_bottom(int width, const char *color) {
    int i;
    printf("   %s╚", color);
    for (i = 0; i < width - 2; i++) printf("═");
    printf("╝%s\n", RST);
}

void draw_box_middle(int width, const char *color) {
    int i;
    printf("   %s╠", color);
    for (i = 0; i < width - 2; i++) printf("═");
    printf("╣%s\n", RST);
}

void draw_box_line(int width, const char *color, const char *text) {
    int text_len = strlen(text);
    int padding = (width - 4 - text_len) / 2;
    int i;
    printf("   %s║%s", color, RST);
    for (i = 0; i < padding; i++) printf(" ");
    printf("%s", text);
    for (i = 0; i < width - 4 - padding - text_len; i++) printf(" ");
    printf("  %s║%s\n", color, RST);
}

void show_msg(char *msg, int type) {
    printf("\n");
    if (type == 1) {
        draw_box_top(44, GRN);
        printf("   %s║%s  %s✓ SUCCESS:%s %-27s%s║%s\n", GRN, RST, BGRN, RST, msg, GRN, RST);
        draw_box_bottom(44, GRN);
        sound_success();
    } else if (type == 2) {
        draw_box_top(44, RED);
        printf("   %s║%s  %s✗ ERROR:%s %-29s%s║%s\n", RED, RST, BRED, RST, msg, RED, RST);
        draw_box_bottom(44, RED);
        sound_error();
    } else if (type == 3) {
        draw_box_top(44, YLW);
        printf("   %s║%s  %s⚠ WARNING:%s %-27s%s║%s\n", YLW, RST, BYLW, RST, msg, YLW, RST);
        draw_box_bottom(44, YLW);
        sound_click();
    } else {
        draw_box_top(44, CYN);
        printf("   %s║%s  %sℹ INFO:%s %-30s%s║%s\n", CYN, RST, BCYN, RST, msg, CYN, RST);
        draw_box_bottom(44, CYN);
        sound_pop();
    }
}

void get_today(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    sprintf(buf, "%d-%02d-%02d", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
}

void get_current_time(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    sprintf(buf, "%02d:%02d:%02d", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
}

void print_slow(char *text, int speed) {
    int i, len = strlen(text);
    for (i = 0; i < len; i++) {
        printf("%c", text[i]);
        fflush(stdout);
        delay_ms(speed);
    }
}

void print_line_animated(char *line, int speed) {
    if (animation_enabled) {
        print_slow(line, speed);
    } else {
        printf("%s", line);
    }
}

void loading(char *text, int time_ms) {
    int i, steps = time_ms / 100;
    printf("   %s⟳%s %s", CYN, RST, text);
    fflush(stdout);
    for (i = 0; i < steps; i++) {
        printf("%s.%s", YLW, RST);
        fflush(stdout);
        delay_ms(100);
    }
    printf(" %s✓%s\n", GRN, RST);
    sound_pop();
}

void progress(char *text, int time_ms) {
    int i, width = 25, step_time = time_ms / width;
    printf("   %s⟳%s %s\n", CYN, RST, text);
    printf("   %s[%s", DIM, RST);
    fflush(stdout);
    for (i = 0; i < width; i++) {
        printf("%s█%s", GRN, RST);
        fflush(stdout);
        delay_ms(step_time);
    }
    printf("%s]%s %s100%%%s\n", DIM, RST, BGRN, RST);
    sound_click();
}

void animate_panel_entrance(const char *color, int width) {
    int i, j;
    if (!animation_enabled) return;
    
    for (i = 0; i < 3; i++) {
        printf("\r   %s", color);
        for (j = 0; j < (i + 1) * (width / 3); j++) {
            printf("▓");
        }
        printf("%s", RST);
        fflush(stdout);
        delay_ms(30);
    }
    printf("\r");
    for (i = 0; i < width + 3; i++) printf(" ");
    printf("\r");
}

void show_splash_screen() {
    clear_screen();
    play_sound(SND_STARTUP);
    printf("%s", BCYN);
    printf("\n\n");
    print_line_animated("   ╔═══════════════════════════════════════════════════════════════════╗\n", 8);
    print_line_animated("   ║                                                                   ║\n", 8);
    print_line_animated("   ║    ███████╗██╗     ███████╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗║\n", 8);
    print_line_animated("   ║    ██╔════╝██║     ██╔════╝██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║║\n", 8);
    print_line_animated("   ║    █████╗  ██║     █████╗  ██║        ██║   ██║██║   ██║██╔██╗ ██║║\n", 8);
    print_line_animated("   ║    ██╔══╝  ██║     ██╔══╝  ██║        ██║   ██║██║   ██║██║╚██╗██║║\n", 8);
    print_line_animated("   ║    ███████╗███████╗███████╗╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║║\n", 8);
    print_line_animated("   ║    ╚══════╝╚══════╝╚══════╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝║\n", 8);
    print_line_animated("   ║                                                                   ║\n", 8);
    print_line_animated("   ║              M A N A G E M E N T   S Y S T E M                    ║\n", 8);
    print_line_animated("   ║                                                                   ║\n", 8);
    print_line_animated("   ╚═══════════════════════════════════════════════════════════════════╝\n", 8);
    printf("%s", RST);
    play_sound(SND_CLICK);
    delay_ms(300);
    printf("\n");
    progress("Initializing Core Modules", 400);
    progress("Loading Security Protocols", 350);
    progress("Configuring Database", 300);
    progress("Starting User Interface", 250);
    printf("\n");
    draw_box_top(44, GRN);
    draw_box_line(44, GRN, "✓ SYSTEM INITIALIZED SUCCESSFULLY");
    draw_box_bottom(44, GRN);
    play_sound(SND_BELL);
    delay_ms(800);
}

void show_header(const char *title, const char *color) {
    int title_len = strlen(title);
    int box_width = title_len + 20;
    if (box_width < 50) box_width = 50;
    if (animation_enabled) {
        animate_box_top(box_width, color);
    } else {
        draw_box_top(box_width, color);
    }
    draw_box_line(box_width, color, title);
    if (animation_enabled) {
        animate_box_bottom(box_width, color);
    } else {
        draw_box_bottom(box_width, color);
    }
}

void show_admin_banner() {
    play_sound(SND_WHOOSH);
    animate_panel_entrance(BYLW, 60);
    printf("%s\n", BYLW);
    print_line_animated("   ╔═══════════════════════════════════════════════════════╗\n", 5);
    print_line_animated("   ║                                                       ║\n", 5);
    print_line_animated("   ║        █████╗ ██████╗ ███╗   ███╗██╗███╗   ██╗        ║\n", 5);
    print_line_animated("   ║       ██╔══██╗██╔══██╗████╗ ████║██║████╗  ██║        ║\n", 5);
    print_line_animated("   ║       ███████║██║  ██║██╔████╔██║██║██╔██╗ ██║        ║\n", 5);
    print_line_animated("   ║       ██╔══██║██║  ██║██║╚██╔╝██║██║██║╚██╗██║        ║\n", 5);
    print_line_animated("   ║       ██║  ██║██████╔╝██║ ╚═╝ ██║██║██║ ╚████║        ║\n", 5);
    print_line_animated("   ║       ╚═╝  ╚═╝╚═════╝ ╚═╝     ╚═╝╚═╝╚═╝  ╚═══╝        ║\n", 5);
    print_line_animated("   ║                                                       ║\n", 5);
    print_line_animated("   ║              ─── Control Panel ───                    ║\n", 5);
    print_line_animated("   ║                                                       ║\n", 5);
    print_line_animated("   ╚═══════════════════════════════════════════════════════╝\n", 5);
    printf("%s", RST);
}

void show_vote_banner() {
    play_sound(SND_WHOOSH);
    animate_panel_entrance(BGRN, 60);
    printf("%s\n", BGRN);
    print_line_animated("   ╔═══════════════════════════════════════════════════════╗\n", 5);
    print_line_animated("   ║                                                       ║\n", 5);
    print_line_animated("   ║       ██╗   ██╗ ██████╗ ████████╗███████╗             ║\n", 5);
    print_line_animated("   ║       ██║   ██║██╔═══██╗╚══██╔══╝██╔════╝             ║\n", 5);
    print_line_animated("   ║       ██║   ██║██║   ██║   ██║   █████╗    NOW!       ║\n", 5);
    print_line_animated("   ║       ╚██╗ ██╔╝██║   ██║   ██║   ██╔══╝               ║\n", 5);
    print_line_animated("   ║        ╚████╔╝ ╚██████╔╝   ██║   ███████╗             ║\n", 5);
    print_line_animated("   ║         ╚═══╝   ╚═════╝    ╚═╝   ╚══════╝             ║\n", 5);
    print_line_animated("   ║                                                       ║\n", 5);
    print_line_animated("   ╚═══════════════════════════════════════════════════════╝\n", 5);
    printf("%s", RST);
}

void show_result_banner() {
    play_sound(SND_WHOOSH);
    animate_panel_entrance(BYLW, 60);
    printf("%s\n", BYLW);
    
    print_line_animated("   ╔═════════════════════════════════════════════════════════════╗\n", 5);
    print_line_animated("   ║                                                             ║\n", 5);
    print_line_animated("   ║   ██████╗ ███████╗███████╗██╗   ██╗██╗  ████████╗███████╗   ║\n", 5);
    print_line_animated("   ║   ██╔══██╗██╔════╝██╔════╝██║   ██║██║  ╚══██╔══╝██╔════╝   ║\n", 5);
    print_line_animated("   ║   ██████╔╝█████╗  ███████╗██║   ██║██║     ██║   ███████╗   ║\n", 5);
    print_line_animated("   ║   ██╔══██╗██╔══╝  ╚════██║██║   ██║██║     ██║   ╚════██║   ║\n", 5);
    print_line_animated("   ║   ██║  ██║███████╗███████║╚██████╔╝███████╗██║   ███████║   ║\n", 5);
    print_line_animated("   ║   ╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝ ╚══════╝╚═╝   ╚══════╝   ║\n", 5);
    print_line_animated("   ║                                                             ║\n", 5);
    print_line_animated("   ╚═════════════════════════════════════════════════════════════╝\n", 5);
    printf("%s", RST);
}

/* ============= ANIMATED PARTY SYMBOLS ============= */

void animate_rice_symbol() {
    int frame, i;
    char *frames[4][8] = {
        {
            "                    ",
            "              %s*%s     ",
            "             %s*%s %s*%s    ",
            "            %s*%s   %s*%s   ",
            "             %s*%s %s*%s    ",
            "              %s│%s     ",
            "              %s│%s     ",
            "          %s◄ RICE ►%s  "
        },
        {
            "            %s*%s   %s*%s   ",
            "             %s*%s %s*%s    ",
            "              %s*%s     ",
            "             %s*%s %s*%s    ",
            "            %s*%s   %s*%s   ",
            "              %s│%s     ",
            "              %s│%s     ",
            "          %s◄ RICE ►%s  "
        },
        {
            "           %s*%s %s*%s %s*%s   ",
            "            %s*%s %s*%s    ",
            "             %s*%s     ",
            "            %s*%s %s*%s    ",
            "           %s*%s %s*%s %s*%s   ",
            "              %s│%s     ",
            "              %s│%s     ",
            "          %s◄ RICE ►%s  "
        },
        {
            "          %s*%s   %s*%s   %s*%s  ",
            "           %s*%s  %s*%s  %s*%s  ",
            "            %s*%s %s*%s %s*%s  ",
            "             %s***%s   ",
            "              %s*%s    ",
            "              %s│%s     ",
            "              %s│%s     ",
            "          %s◄ RICE ►%s  "
        }
    };
    
    printf("\n");
    for (frame = 0; frame < 4; frame++) {
        for (i = 0; i < 8; i++) {
            printf("   ");
            if (i < 5) {
                printf(frames[frame][i], BGRN, RST, BGRN, RST, BGRN, RST, BGRN, RST, BGRN, RST);
            } else if (i < 7) {
                printf(frames[frame][i], YLW, RST);
            } else {
                printf(frames[frame][i], BCYN, RST);
            }
            printf("\n");
        }
        fflush(stdout);
        delay_ms(150);
        if (frame < 3) {
            printf("\033[8A");
        }
    }
    sound_click();
}

void animate_lily_symbol() {
    int frame, i;
    char *petals[] = {BMAG, MAG, BCYN, MAG, BMAG};
    
    printf("\n");
    for (frame = 0; frame < 5; frame++) {
        printf("   %s              ✿%s\n", petals[frame], RST);
        delay_ms(80);
    }
    printf("   %s             ╱ ╲%s\n", MAG, RST); delay_ms(50);
    printf("   %s            ╱   ╲%s\n", MAG, RST); delay_ms(50);
    printf("   %s           ╱%s  %s●%s  %s╲%s\n", MAG, RST, BYLW, RST, MAG, RST); delay_ms(50);
    printf("   %s            ╲   ╱%s\n", MAG, RST); delay_ms(50);
    printf("   %s             ╲ ╱%s\n", MAG, RST); delay_ms(50);
    printf("   %s              │%s\n", GRN, RST); delay_ms(50);
    printf("   %s             ╱│╲%s\n", GRN, RST); delay_ms(50);
    printf("              %s◄ LILY ►%s\n", BCYN, RST);
    sound_click();
}

void animate_scales_symbol() {
    int frame;
    char *tilt[] = {"════╦════", "═══╦═════", "════╦════", "═════╦═══", "════╦════"};
    
    printf("\n");
    for (frame = 0; frame < 5; frame++) {
        printf("   %s         %s%s%s\n", BYLW, tilt[frame], RST, "");
        if (frame < 4) {
            fflush(stdout);
            delay_ms(100);
            printf("\033[1A\r");
        }
    }
    printf("   %s        ╱    ║    ╲%s\n", BYLW, RST); delay_ms(40);
    printf("   %s       ▼     ║     ▼%s\n", BYLW, RST); delay_ms(40);
    printf("   %s      ╭─╮    ║    ╭─╮%s\n", BYLW, RST); delay_ms(40);
    printf("   %s      ╰─╯    ║    ╰─╯%s\n", BYLW, RST); delay_ms(40);
    printf("   %s             ║%s\n", BYLW, RST); delay_ms(40);
    printf("   %s            ╔╩╗%s\n", BYLW, RST); delay_ms(40);
    printf("   %s            ╚═╝%s\n", BYLW, RST);
    printf("           %s◄ SCALES ►%s\n", BCYN, RST);
    sound_click();
}

void animate_pen_symbol() {
    int i;
    char *colors[] = {BLU, BBLU, BLU, BBLU, BLU};
    
    printf("\n");
    for (i = 0; i < 5; i++) {
        printf("   %s              ▲%s\n", colors[i], RST);
        fflush(stdout);
        delay_ms(60);
        if (i < 4) printf("\033[1A\r");
    }
    printf("   %s             ╱█╲%s\n", BLU, RST); delay_ms(40);
    printf("   %s            │███│%s\n", BLU, RST); delay_ms(40);
    printf("   %s            │███│%s\n", BLU, RST); delay_ms(40);
    printf("   %s            │███│%s\n", BLU, RST); delay_ms(40);
    printf("   %s            │███│%s\n", BLU, RST); delay_ms(40);
    printf("   %s            ╰───╯%s\n", BLU, RST);
    printf("              %s◄ PEN ►%s\n", BCYN, RST);
    sound_click();
}

void animate_hand_fan_symbol() {
        printf("\n");
        delay_ms(40);
        printf("             * * * * *\n"); fflush(stdout); delay_ms(40);
        printf("            *  * * *  *\n"); fflush(stdout); delay_ms(40);
        printf("           *   * * *   *\n"); fflush(stdout); delay_ms(40);
        printf("            *  * * *  *\n"); fflush(stdout); delay_ms(40);
        printf("             * * * * *\n"); fflush(stdout); delay_ms(40);
        printf("                 *\n"); fflush(stdout); delay_ms(40);
        printf("                 *\n"); fflush(stdout); delay_ms(40);
        printf("                 *\n"); fflush(stdout);
    printf("              %s◄ HAND FAN ►%s\n", BCYN, RST);
    sound_click();
}

void show_symbol_animated(int sym) {
    printf("%s", BYLW);
    switch(sym) {
        case 1: animate_rice_symbol(); break;
        case 2: animate_lily_symbol(); break;
        case 3: animate_scales_symbol(); break;
        case 4: animate_pen_symbol(); break;
        case 5: animate_hand_fan_symbol(); break;
        default: printf("   Unknown symbol\n"); break;
    }
    printf("%s", RST);
}

void show_trophy_animated() {
    int i, j;
    clear_screen();
    
    /* Sparkle animation */
    for (i = 0; i < 5; i++) {
        clear_screen();
        printf("\n\n\n\n\n");
        printf("%s", (i % 2 == 0) ? BYLW : BGRN);
        printf("         ");
        for (j = 0; j < 10; j++) {
            printf("%s ", (rand() % 2 == 0) ? "✦" : "★");
        }
        printf("%s\n", RST);
        fflush(stdout);
        delay_ms(100);
    }
    
    /* Trophy reveal animation */
    clear_screen();
    printf("%s\n\n", BYLW);
    print_line_animated("            ╔═════════════════════╗\n", 15);
    print_line_animated("            ║    ★ CHAMPION ★    ║\n", 15);
    print_line_animated("            ╠═════════════════════╣\n", 15);
    print_line_animated("            ║        ╭───╮        ║\n", 15);
    print_line_animated("            ║       ╱     ╲       ║\n", 15);
    print_line_animated("            ║      │   ♕   │      ║\n", 15);
    print_line_animated("            ║       ╲     ╱       ║\n", 15);
    print_line_animated("            ║        ╰┬─┬╯        ║\n", 15);
    print_line_animated("            ║         │ │         ║\n", 15);
    print_line_animated("            ║        ╭┴─┴╮        ║\n", 15);
    print_line_animated("            ║       ╱     ╲       ║\n", 15);
    print_line_animated("            ╚═════════════════════╝\n", 15);
    printf("%s", RST);
    
    sound_winner();
    
    /* Blinking winner text */
    for (i = 0; i < 8; i++) {
        printf("\r            %s★★★ WINNER ★★★%s", (i % 2 == 0) ? BGRN : BYLW, RST);
        fflush(stdout);
        delay_ms(150);
    }
    printf("\n\n");
    
    draw_box_top(44, BGRN);
    draw_box_line(44, BGRN, "🎉 CONGRATULATIONS! 🎉");
    draw_box_line(44, BGRN, "ELECTION WINNER DECLARED!");
    draw_box_bottom(44, BGRN);
}

void clear_all_data() {
    memset(&election, 0, sizeof(election));
    memset(party, 0, sizeof(party));
    memset(voter, 0, sizeof(voter));
    memset(area, 0, sizeof(area));
    memset(center, 0, sizeof(center));
    memset(&admin_security, 0, sizeof(admin_security));
}

void load_parties() {
    if (election.party_count > 0) return;
    strcpy(party[0].name, "BNP");
    party[0].id = 1;
    party[0].symbol = 1;
    party[0].active = 1;
    party[0].total = 0;
    strcpy(party[0].leader, "Tarique Rahman");
    strcpy(party[0].slogan, "Democracy for All");
    
    strcpy(party[1].name, "NCP");
    party[1].id = 2;
    party[1].symbol = 2;
    party[1].active = 1;
    party[1].total = 0;
    strcpy(party[1].leader, "National Leader");
    strcpy(party[1].slogan, "Progress & Unity");
    
    strcpy(party[2].name, "JAMAT");
    party[2].id = 3;
    party[2].symbol = 3;
    party[2].active = 1;
    party[2].total = 0;
    strcpy(party[2].leader, "Islamic Leader");
    strcpy(party[2].slogan, "Justice for Nation");
    
    strcpy(party[3].name, "INDEPENDENT");
    party[3].id = 4;
    party[3].symbol = 4;
    party[3].active = 1;
    party[3].total = 0;
    strcpy(party[3].leader, "Independent");
    strcpy(party[3].slogan, "Voice of People");
    
    strcpy(party[4].name, "JSD");
    party[4].id = 5;
    party[4].symbol = 5;
    party[4].active = 1;
    party[4].total = 0;
    strcpy(party[4].leader, "Socialist Leader");
    strcpy(party[4].slogan, "Workers United");
    
    election.party_count = 5;
    strcpy(election.name, "Bangladesh National Election 2026");
    strcpy(election.date, "2026-01-20");
    strcpy(election.start_time, "08:00");
    strcpy(election.end_time, "17:00");
}

void save_data() {
    FILE *f = fopen("election_data.dat", "wb");
    if (f == NULL) {
        show_msg("Failed to save data!", 2);
        return;
    }
    fwrite(&election, sizeof(election), 1, f);
    fwrite(party, sizeof(party), 1, f);
    fwrite(voter, sizeof(voter), 1, f);
    fwrite(area, sizeof(area), 1, f);
    fwrite(center, sizeof(center), 1, f);
    fwrite(&admin_security, sizeof(admin_security), 1, f);
    fwrite(&audit_count, sizeof(audit_count), 1, f);
    fwrite(audit_log, sizeof(audit_log), 1, f);
    fclose(f);
}

void load_data() {
    FILE *f = fopen("election_data.dat", "rb");
    if (f == NULL) {
        load_parties();
        return;
    }
    fread(&election, sizeof(election), 1, f);
    fread(party, sizeof(party), 1, f);
    fread(voter, sizeof(voter), 1, f);
    fread(area, sizeof(area), 1, f);
    fread(center, sizeof(center), 1, f);
    fread(&admin_security, sizeof(admin_security), 1, f);
    fread(&audit_count, sizeof(audit_count), 1, f);
    fread(audit_log, sizeof(audit_log), 1, f);
    fclose(f);
}

void save_security_data() {
    FILE *f = fopen("security.dat", "wb");
    if (f == NULL) return;
    fwrite(&admin_security, sizeof(admin_security), 1, f);
    fclose(f);
}

void load_security_data() {
    FILE *f = fopen("security.dat", "rb");
    if (f == NULL) {
        admin_security.failed_attempts = 0;
        admin_security.lockout_time = 0;
        admin_security.is_locked = 0;
        return;
    }
    fread(&admin_security, sizeof(admin_security), 1, f);
    fclose(f);
}

void show_lockout_screen(int remaining) {
    int i;
    clear_screen();
    printf("%s\n\n", BRED);
    print_line_animated("   ╔═══════════════════════════════════════════════════════╗\n", 3);
    printf("   ║                                                       ║\n");
    printf("   ║      ██╗      ██████╗  ██████╗██╗  ██╗███████╗██████╗ ║\n");
    printf("   ║      ██║     ██╔═══██╗██╔════╝██║ ██╔╝██╔════╝██╔══██╗║\n");
    printf("   ║      ██║     ██║   ██║██║     █████╔╝ █████╗  ██║  ██║║\n");
    printf("   ║      ██║     ██║   ██║██║     ██╔═██╗ ██╔══╝  ██║  ██║║\n");
    printf("   ║      ███████╗╚██████╔╝╚██████╗██║  ██╗███████╗██████╔╝║\n");
    printf("   ║      ╚══════╝ ╚═════╝  ╚═════╝╚═╝  ╚═╝╚══════╝╚═════╝ ║\n");
    printf("   ║                                                       ║\n");
    printf("   ╠═══════════════════════════════════════════════════════╣\n");
    printf("   ║                                                       ║\n");
    printf("   ║   %s⚠  ADMIN ACCOUNT TEMPORARILY LOCKED  ⚠%s             ║\n", BYLW, BRED);
    printf("   ║                                                       ║\n");
    printf("   ║   Too many failed login attempts detected.            ║\n");
    printf("   ║   Please wait for the lockout period to expire.       ║\n");
    printf("   ║                                                       ║\n");
    print_line_animated("   ╚═══════════════════════════════════════════════════════╝\n", 3);
    printf("%s\n", RST);
    
    printf("   %s┌─────────────────────────────────────────┐%s\n", YLW, RST);
    printf("   %s│%s  Time Remaining: %s%02d:%02d%s                    %s│%s\n", 
           YLW, RST, BRED, remaining / 60, remaining % 60, RST, YLW, RST);
    printf("   %s│%s                                         %s│%s\n", YLW, RST, YLW, RST);
    printf("   %s│%s  Progress: [", YLW, RST);
    
    int progress_width = 20;
    int filled = (int)((float)(LOCKOUT_SECONDS - remaining) / LOCKOUT_SECONDS * progress_width);
    for (i = 0; i < filled; i++) printf("%s█%s", GRN, RST);
    for (i = filled; i < progress_width; i++) printf("%s░%s", DIM, RST);
    printf("]     %s│%s\n", YLW, RST);
    printf("   %s└─────────────────────────────────────────┘%s\n", YLW, RST);
}

int check_admin_lockout() {
    load_security_data();
    if (admin_security.is_locked) {
        time_t current_time = time(NULL);
        int elapsed = (int)(current_time - admin_security.lockout_time);
        if (elapsed >= LOCKOUT_SECONDS) {
            admin_security.is_locked = 0;
            admin_security.failed_attempts = 0;
            admin_security.lockout_time = 0;
            save_security_data();
            return 0;
        }
        int remaining = LOCKOUT_SECONDS - elapsed;
        while (remaining > 0) {
            show_lockout_screen(remaining);
            delay_ms(1000);
            remaining--;
            if (remaining <= 0) {
                admin_security.is_locked = 0;
                admin_security.failed_attempts = 0;
                admin_security.lockout_time = 0;
                save_security_data();
                clear_screen();
                show_msg("Account unlocked!", 1);
                delay_ms(1000);
                return 0;
            }
        }
    }
    return 0;
}

int admin_login() {
    char pw[30];
    int i;
    char c;
    
    load_security_data();
    if (admin_security.is_locked) {
        check_admin_lockout();
    }
    
    int remaining_attempts = MAX_LOGIN_ATTEMPTS - admin_security.failed_attempts;
    
    while (remaining_attempts > 0) {
        clear_screen();
        sound_whoosh();
        animate_panel_entrance(BYLW, 60);
        
        printf("%s\n", BYLW);
        print_line_animated("   ╔═══════════════════════════════════════════════════════╗\n", 5);
        printf("   ║                                                       ║\n");
        printf("   ║           🔐 ADMIN AUTHENTICATION 🔐                  ║\n");
        printf("   ║                                                       ║\n");
        printf("   ╠═══════════════════════════════════════════════════════╣\n");
        printf("   ║                                                       ║\n");
        printf("   ║   Enter your credentials to access the admin panel    ║\n");
        printf("   ║                                                       ║\n");
        print_line_animated("   ╚═══════════════════════════════════════════════════════╝\n", 5);
        printf("%s\n", RST);
        
        printf("   %s╔═════════════════════════════════════════╗%s\n", CYN, RST);
        printf("   %s║%s  Attempts Remaining: ", CYN, RST);
        if (remaining_attempts > 1) {
            printf("%s%d%s                    ", BGRN, remaining_attempts, RST);
        } else {
            printf("%s%d ⚠%s                  ", BRED, remaining_attempts, RST);
        }
        printf("%s║%s\n", CYN, RST);
        printf("   %s║%s                                         %s║%s\n", CYN, RST, CYN, RST);
        printf("   %s║%s  [", CYN, RST);
        for (i = 0; i < MAX_LOGIN_ATTEMPTS; i++) {
            if (i < remaining_attempts) printf("%s●%s", BGRN, RST);
            else printf("%s○%s", RED, RST);
        }
        printf("]                                  %s║%s\n", CYN, RST);
        printf("   %s╚═════════════════════════════════════════╝%s\n", CYN, RST);
        
        printf("\n   %s🔑 Password:%s ", YLW, RST);
        
        i = 0;
        memset(pw, 0, sizeof(pw));
        while (1) {
            c = getch_custom();
            if (c == '\n' || c == '\r') break;
            else if (c == 127 || c == 8) {
                if (i > 0) {
                    i--;
                    pw[i] = '\0';
                    printf("\b \b");
                    fflush(stdout);
                }
            } else if (i < 28) {
                pw[i] = c;
                i++;
                printf("%s•%s", GRN, RST);
                fflush(stdout);
            }
        }
        printf("\n\n");
        
        loading("Verifying credentials", 500);
        
        if (strcmp(pw, PASSWORD) == 0) {
            admin_security.failed_attempts = 0;
            admin_security.is_locked = 0;
            save_security_data();
            add_audit_log("Admin login successful", "ADMIN");
            show_msg("Login Successful!", 1);
            sound_coin();
            delay_ms(800);
            return 1;
        }
        
        admin_security.failed_attempts++;
        remaining_attempts = MAX_LOGIN_ATTEMPTS - admin_security.failed_attempts;
        add_audit_log("Failed admin login attempt", "UNKNOWN");
        
        if (remaining_attempts <= 0) {
            admin_security.is_locked = 1;
            admin_security.lockout_time = time(NULL);
            save_security_data();
            play_sound(SND_LOCK);
            
            clear_screen();
            printf("%s\n\n", BRED);
            printf("   ╔═══════════════════════════════════════════════════════╗\n");
            printf("   ║                                                       ║\n");
            printf("   ║      ⚠️  SECURITY ALERT: ACCOUNT LOCKED ⚠️              ║\n");
            printf("   ║                                                       ║\n");
            printf("   ║   Maximum login attempts exceeded!                    ║\n");
            printf("   ║   Account locked for %d seconds.                      ║\n", LOCKOUT_SECONDS);
            printf("   ║                                                       ║\n");
            printf("   ║   This incident has been logged.                      ║\n");
            printf("   ║                                                       ║\n");
            printf("   ╚═══════════════════════════════════════════════════════╝\n");
            printf("%s\n", RST);
            add_audit_log("Admin account LOCKED - Max attempts exceeded", "SECURITY");
            wait_key();
            return 0;
        }
        
        show_msg("Invalid password!", 2);
        delay_ms(1000);
    }
    return 0;
}

void show_about() {
    clear_screen();
    play_sound(SND_INTRO);
    animate_panel_entrance(BMAG, 70);
    
    printf("%s", BMAG);
    printf("\n");
    printf("   %s╔════════════════════════════════════════════════════════════╗%s\n", BMAG, RST);
    printf("   %s║%s                                                            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        █████╗ ██████╗  ██████╗ ██╗   ██╗████████╗          %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        ██╔══██╗██╔══██╗██╔═══██╗██║   ██║╚══██╔══╝         %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        ███████║██████╔╝██║   ██║██║   ██║   ██║            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        ██╔══██║██╔══██╗██║   ██║██║   ██║   ██║            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        ██║  ██║██████╔╝╚██████╔╝╚██████╔╝   ██║            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s        ╚═╝  ╚═╝╚═════╝  ╚═════╝  ╚═════╝    ╚═╝            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s║%s                                                            %s║%s\n", BMAG, RST, BMAG, RST);
    printf("   %s╚════════════════════════════════════════════════════════════╝%s\n", BMAG, RST);
    printf("\n");
    
    delay_ms(800);
    printf("   %s╔════════════════════════[ PROJECT INFO ]════════════════════════╗%s\n", CYN, RST);
    printf("   %s║%s                                                                %s║%s\n", CYN, RST, CYN, RST);

    printf("   %s║%s  %sProject%s     : ", CYN, RST, YLW, RST);
    print_slow("Election Management System", 12);
    printf("                      %s║%s\n", CYN, RST);

    printf("   %s║%s  %sLanguage%s    : ", CYN, RST, YLW, RST);
    print_slow("C Programming", 12);
    printf("                                   %s║%s\n", CYN, RST);

    printf("   %s║%s  %sCourse%s      : ", CYN, RST, YLW, RST);
    print_slow("CSE", 10);
    printf("                                             %s║%s\n", CYN, RST);

    printf("   %s║%s  %sSemester%s    : ", CYN, RST, YLW, RST);
    print_slow("1st Semester", 12);
    printf("                                    %s║%s\n", CYN, RST);

    printf("   %s║%s  %sSection%s     : ", CYN, RST, YLW, RST);
    print_slow("1CM", 12);
    printf("                                             %s║%s\n", CYN, RST);

    printf("   %s║%s                                                                %s║%s\n", CYN, RST, CYN, RST);
    printf("   %s╚════════════════════════════════════════════════════════════════╝%s\n", CYN, RST);

    play_sound(SND_CLICK);
    delay_ms(600);
    printf("\n");

    printf("   %s╔═════════════════════════[ UNIVERSITY ]═════════════════════════╗%s\n", BLU, RST);
    printf("   %s║%s                                                                %s║%s\n", BLU, RST, BLU, RST);
    
    printf("   %s║%s           %sInternational Islamic University Chittagong%s          %s║%s\n", BLU, RST, WHT, RST, BLU, RST);
    printf("   %s║%s                             %s(IIUC)%s                             %s║%s\n", BLU, RST, BCYN, RST, BLU, RST);
    
    printf("   %s║%s                                                                %s║%s\n", BLU, RST, BLU, RST);
    
    printf("   %s║%s                 %sDepartment of Computer Science%s                 %s║%s\n", BLU, RST, GRN, RST, BLU, RST);
    printf("   %s║%s                          %s& Engineering%s                         %s║%s\n", BLU, RST, GRN, RST, BLU, RST);
    
    printf("   %s║%s                                                                %s║%s\n", BLU, RST, BLU, RST);
    printf("   %s╚════════════════════════════════════════════════════════════════╝%s\n", BLU, RST);

    play_sound(SND_CLICK);
    delay_ms(600);
    printf("\n");

    printf("   %s╔════════════════════════[ DEVELOPED BY ]════════════════════════╗%s\n", GRN, RST);
    printf("   %s║%s                                                                %s║%s\n", GRN, RST, GRN, RST);

    printf("   %s║%s    %s╔══════════════════════════════════════════════════════╗%s    %s║%s\n", GRN, RST, BYLW, RST, GRN, RST);
    printf("   %s║%s    %s║%s     %s[1] MEMBER 1 - TEAM LEAD & VISUAL ARCHITECT%s      %s║%s    %s║%s\n", GRN, RST, BYLW, RST, WHT, RST, BYLW, RST, GRN, RST);
    printf("   %s║%s    %s╠──────────────────────────────────────────────────────╣%s    %s║%s\n", GRN, RST, BYLW, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sName%s : ", GRN, RST, BYLW, RST, CYN, RST);
    print_slow("MD Ataur Rahman Sifat", 15);
    printf("                         %s║%s    %s║%s\n", BYLW, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sID%s   : ", GRN, RST, BYLW, RST, CYN, RST);
    print_slow("C253063", 15);
    printf("                                       %s║%s    %s║%s\n", BYLW, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sRole%s : UI/UX, ASCII Art, Menus, Visual Effects       %s║%s    %s║%s\n", GRN, RST, BYLW, RST, CYN, RST, BYLW, RST, GRN, RST);
    
    printf("   %s║%s    %s╚══════════════════════════════════════════════════════╝%s    %s║%s\n", GRN, RST, BYLW, RST, GRN, RST);
    play_sound(SND_POP);
    delay_ms(800);

    printf("   %s║%s                                                                %s║%s\n", GRN, RST, GRN, RST);

    printf("   %s║%s    %s╔══════════════════════════════════════════════════════╗%s    %s║%s\n", GRN, RST, BCYN, RST, GRN, RST);
    printf("   %s║%s    %s║%s       %s[2] MEMBER 2 - DATABASE ADMIN & BACKEND%s        %s║%s    %s║%s\n", GRN, RST, BCYN, RST, WHT, RST, BCYN, RST, GRN, RST);
    printf("   %s║%s    %s╠──────────────────────────────────────────────────────╣%s    %s║%s\n", GRN, RST, BCYN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sName%s : ", GRN, RST, BCYN, RST, CYN, RST);
    print_slow("Muhammad Minhazul Abedin", 15);
    printf("                      %s║%s    %s║%s\n", BCYN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sID%s   : ", GRN, RST, BCYN, RST, CYN, RST);
    print_slow("C253100", 15);
    printf("                                       %s║%s    %s║%s\n", BCYN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sRole%s : Data Structures, File I/O, Admin Panel        %s║%s    %s║%s\n", GRN, RST, BCYN, RST, CYN, RST, BCYN, RST, GRN, RST);
    
    printf("   %s║%s    %s╚══════════════════════════════════════════════════════╝%s    %s║%s\n", GRN, RST, BCYN, RST, GRN, RST);
    play_sound(SND_POP);
    delay_ms(800);

    printf("   %s║%s                                                                %s║%s\n", GRN, RST, GRN, RST);

    printf("   %s║%s    %s╔══════════════════════════════════════════════════════╗%s    %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s    %s║%s      %s[3] MEMBER 3 - LOGIC CONTROLLER & SECURITY%s      %s║%s    %s║%s\n", GRN, RST, BGRN, RST, WHT, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s    %s╠──────────────────────────────────────────────────────╣%s    %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sName%s : ", GRN, RST, BGRN, RST, CYN, RST);
    print_slow("Tajmul Hasan Chowdhury", 15);
    printf("                        %s║%s    %s║%s\n", BGRN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sID%s   : ", GRN, RST, BGRN, RST, CYN, RST);
    print_slow("C253066", 15);
    printf("                                       %s║%s    %s║%s\n", BGRN, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sRole%s : Registration, Geo-Fence, Validation           %s║%s    %s║%s\n", GRN, RST, BGRN, RST, CYN, RST, BGRN, RST, GRN, RST);
    
    printf("   %s║%s    %s╚══════════════════════════════════════════════════════╝%s    %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    play_sound(SND_POP);
    delay_ms(800);

    printf("   %s║%s                                                                %s║%s\n", GRN, RST, GRN, RST);

    printf("   %s║%s    %s╔══════════════════════════════════════════════════════╗%s    %s║%s\n", GRN, RST, BMAG, RST, GRN, RST);
    printf("   %s║%s    %s║%s   %s[4] MEMBER 4 - ALGORITHM SPECIALIST & MULTIMEDIA%s   %s║%s    %s║%s\n", GRN, RST, BMAG, RST, WHT, RST, BMAG, RST, GRN, RST);
    printf("   %s║%s    %s╠──────────────────────────────────────────────────────╣%s    %s║%s\n", GRN, RST, BMAG, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sName%s : ", GRN, RST, BMAG, RST, CYN, RST);
    print_slow("Noushad Naser Noyon", 15);
    printf("                           %s║%s    %s║%s\n", BMAG, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sID%s   : ", GRN, RST, BMAG, RST, CYN, RST);
    print_slow("C253116", 15);
    printf("                                       %s║%s    %s║%s\n", BMAG, RST, GRN, RST);

    printf("   %s║%s    %s║%s %sRole%s : Sound System, Results, Animations             %s║%s    %s║%s\n", GRN, RST, BMAG, RST, CYN, RST, BMAG, RST, GRN, RST);
    
    printf("   %s║%s    %s╚══════════════════════════════════════════════════════╝%s    %s║%s\n", GRN, RST, BMAG, RST, GRN, RST);
    play_sound(SND_POP);
    delay_ms(800);

    printf("   %s║%s                                                                %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚════════════════════════════════════════════════════════════════╝%s\n", GRN, RST);

    printf("\n");
    printf("   %s╔══════════════════════════════════════════════════════════════════╗%s\n", MAG, RST);
    printf("   %s║%s           %s(C) 2026 IIUC CSE 1CM - All Rights Reserved%s            %s║%s\n", MAG, RST, WHT, RST, MAG, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════════╝%s\n", MAG, RST);

    play_sound(SND_SUCCESS);
    wait_key();
}

int main_menu() {
    int choice;
    char time_buf[10], date_buf[15];
    
    clear_screen();
    get_today(date_buf);
    get_current_time(time_buf);
    
    animate_panel_entrance(BBLU, 74);
    
    printf("%s\n", BBLU);
    print_line_animated("   ╔═══════════════════════════════════════════════════════════════════════╗\n", 3);
    printf("   ║                                                                       ║\n");
    printf("   ║      ███████╗██╗     ███████╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗  ║\n");
    printf("   ║      ██╔════╝██║     ██╔════╝██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║  ║\n");
    printf("   ║      █████╗  ██║     █████╗  ██║        ██║   ██║██║   ██║██╔██╗ ██║  ║\n");
    printf("   ║      ██╔══╝  ██║     ██╔══╝  ██║        ██║   ██║██║   ██║██║╚██╗██║  ║\n");
    printf("   ║      ███████╗███████╗███████╗╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║  ║\n");
    printf("   ║      ╚══════╝╚══════╝╚══════╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝  ║\n");
    printf("   ║                                                                       ║\n");
    printf("   ║                   M A N A G E M E N T   S Y S T E M                   ║\n");
    printf("   ║                                                                       ║\n");
    print_line_animated("   ╚═══════════════════════════════════════════════════════════════════════╝\n", 3);
    printf("%s\n", RST);
    
    printf("   %s╔═══════════════════════════════════════════════════════════════════════╗%s\n", DIM, RST);
    printf("   %s║%s  📅 %s  │  ⏰ %s  │  Status: %s%s%s                    %s║%s\n", 
        DIM, RST, date_buf, time_buf, 
        election.is_running ? BGRN : BRED,
        election.is_running ? "RUNNING" : "STOPPED",
        RST, DIM, RST);
    printf("   %s╚═══════════════════════════════════════════════════════════════════════╝%s\n", DIM, RST);
    printf("\n");
    
    printf("   %s╔═════════════════════════════[ MAIN MENU ]═════════════════════════════╗%s\n", BYLW, RST);
    printf("   %s║%s                                                                       %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s║%s   %s[1]%s  🔐  Admin Control Panel                                        %s║%s\n", BYLW, RST, RED, RST, BYLW, RST);
    printf("   %s║%s   %s[2]%s  👤  Voter Registration & Status                                %s║%s\n", BYLW, RST, CYN, RST, BYLW, RST);
    printf("   %s║%s   %s[3]%s  🗳️   Cast Your Vote                                             %s║%s\n", BYLW, RST, GRN, RST, BYLW, RST);
    printf("   %s║%s   %s[4]%s  📊  View Election Results                                      %s║%s\n", BYLW, RST, MAG, RST, BYLW, RST);
    printf("   %s║%s   %s[5]%s  📈  Live Statistics Dashboard                                  %s║%s\n", BYLW, RST, BLU, RST, BYLW, RST);
    printf("   %s║%s   %s[6]%s  ⚙️   System Settings                                            %s║%s\n", BYLW, RST, CYN, RST, BYLW, RST);
    printf("   %s║%s   %s[7]%s  ℹ️   About This Project                                         %s║%s\n", BYLW, RST, BCYN, RST, BYLW, RST);
    printf("   %s║%s   %s[8]%s  🚪  Exit System                                                %s║%s\n", BYLW, RST, YLW, RST, BYLW, RST);
    printf("   %s║%s                                                                       %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s╚═══════════════════════════════════════════════════════════════════════╝%s\n", BYLW, RST);
    
    printf("\n   %s▶%s Enter choice [1-8]: ", BGRN, RST);
    if (get_int("", &choice, 1, 8) == 0) return 0;
    sound_click();
    return choice;
}

int admin_menu() {
    int choice;
    clear_screen();
    show_admin_banner();
    printf("\n");
    
    printf("   %s╔═══════════════════════════[ ADMIN MENU ]═════════════════════════╗%s\n", BYLW, RST);
    printf("   %s║%s                                                                  %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s║%s   %s[1]%s  📝  Election Setup & Configuration                        %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[2]%s  🏛️   View All Political Parties                            %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[3]%s  🗺️   Manage Voting Areas                                   %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[4]%s  🏢  Manage Voting Centers                                 %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[5]%s  👥  View Registered Voters                                %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[6]%s  ⚡  Election Start/Stop Control                           %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
    printf("   %s║%s   %s[7]%s  📜  View Audit Logs                                       %s║%s\n", BYLW, RST, BCYN, RST, BYLW, RST);
    printf("   %s║%s   %s[8]%s  🔄  Reset All Election Data                               %s║%s\n", BYLW, RST, BRED, RST, BYLW, RST);
    printf("   %s║%s   %s[9]%s  🚪  Logout                                                %s║%s\n", BYLW, RST, YLW, RST, BYLW, RST);
    printf("   %s║%s                                                                  %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════════╝%s\n", BYLW, RST);
    
    printf("\n   %s▶%s Choice: ", BGRN, RST);
    if (get_int("", &choice, 1, 9) == 0) return 0;
    play_sound(SND_CLICK);
    return choice;
}

void admin_setup() {
    char buf[50];
    clear_screen();
    sound_whoosh();
    show_header("ELECTION SETUP & CONFIGURATION", BCYN);
    printf("\n");
    
    printf("   %s┌─────────────────[ Current Settings ]─────────────────┐%s\n", CYN, RST);
    printf("   %s│%s                                                      %s│%s\n", CYN, RST, CYN, RST);
    printf("   %s│%s  Election Name : %-35s %s│%s\n", CYN, RST, election.name, CYN, RST);
    printf("   %s│%s  Election Date : %-35s %s│%s\n", CYN, RST, election.date, CYN, RST);
    printf("   %s│%s  Start Time    : %-35s %s│%s\n", CYN, RST, election.start_time, CYN, RST);
    printf("   %s│%s  End Time      : %-35s %s│%s\n", CYN, RST, election.end_time, CYN, RST);
    printf("   %s│%s                                                      %s│%s\n", CYN, RST, CYN, RST);
    printf("   %s└──────────────────────────────────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   %s(Press Enter to keep current value)%s\n\n", DIM, RST);
    
    printf("   New Election Name: ");
    if (get_string("", buf, 50)) strncpy(election.name, buf, 49);
    
    printf("   New Election Date (YYYY-MM-DD): ");
    if (get_string("", buf, 15)) strncpy(election.date, buf, 14);
    
    printf("   Start Time (HH:MM): ");
    if (get_string("", buf, 10)) strncpy(election.start_time, buf, 9);
    
    printf("   End Time (HH:MM): ");
    if (get_string("", buf, 10)) strncpy(election.end_time, buf, 9);
    
    progress("Saving configuration", 400);
    save_data();
    add_audit_log("Election settings updated", "ADMIN");
    show_msg("Setup completed!", 1);
    wait_key();
}

void admin_parties() {
    int i;
    char ch;
    char *sym_names[] = {"", "Rice", "Lily", "Scales", "Pen", "Hand Fan"};
    char *colors[] = {"", GRN, CYN, YLW, MAG, RED};
    
    clear_screen();
    sound_whoosh();
    show_header("POLITICAL PARTIES OVERVIEW", BCYN);
    printf("\n");
    
    printf("   %s╔══════╦═══════════════╦════════════╦═════════╦════════════════════╗%s\n", CYN, RST);
    printf("   %s║%s %sID%s   %s║%s %sPARTY%s         %s║%s %sSYMBOL%s     %s║%s %sVOTES%s   %s║%s %sLEADER%s             %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠══════╬═══════════════╬════════════╬═════════╬════════════════════╣%s\n", CYN, RST);
    
    for (i = 0; i < election.party_count; i++) {
        printf("   %s║%s  %s%d%s   %s║%s %s%-13s%s %s║%s %-10s %s║%s  %s%5d%s  %s║%s %-18s %s║%s\n",
            CYN, RST, colors[i+1], party[i].id, RST, CYN, RST,
            colors[i+1], party[i].name, RST, CYN, RST,
            sym_names[party[i].symbol], CYN, RST,
            BGRN, party[i].total, RST, CYN, RST,
            party[i].leader, CYN, RST);
        printf("   %s╠──────┼───────────────┼────────────┼─────────┼────────────────────╣%s\n", CYN, RST);
        sound_pop();
        delay_ms(800);
    }
    printf("   %s╚══════╩═══════════════╩════════════╩═════════╩════════════════════╝%s\n", CYN, RST);
    
    printf("\n   %sShow party symbols? (y/n):%s ", YLW, RST);
    ch = getch_custom();
    printf("%c\n", ch);
    
    if (ch == 'y' || ch == 'Y') {
        for (i = 0; i < election.party_count; i++) {
            printf("\n   %s[%d] %s%s%s - \"%s\"\n", CYN, i + 1, colors[i+1], party[i].name, RST, party[i].slogan);
            show_symbol_animated(party[i].symbol);
            delay_ms(300);
        }
    }
    wait_key();
}

int area_menu() {
    int choice;
    clear_screen();
    sound_whoosh();
    show_header("AREA MANAGEMENT", BCYN);
    
    printf("\n   %sTotal Areas:%s %s%d%s / %d\n", YLW, RST, BGRN, election.area_count, RST, MAX_C);
    printf("\n");
    
    printf("   %s┌──────────────────────────────────────┐%s\n", GRN, RST);
    printf("   %s│%s  %s[1]%s  ➕ Add New Area                %s│%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s│%s  %s[2]%s  📋 View All Areas              %s│%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s│%s  %s[3]%s  🔙 Back                        %s│%s\n", GRN, RST, YLW, RST, GRN, RST);
    printf("   %s└──────────────────────────────────────┘%s\n", GRN, RST);
    
    printf("\n   %s▶%s Choice: ", BGRN, RST);
    if (get_int("", &choice, 1, 3) == 0) return 0;
    sound_click();
    return choice;
}

void add_area() {
    char name_buf[50];
    int i;
    char cf;
    
    clear_screen();
    sound_whoosh();
    show_header("ADD NEW VOTING AREA", BGRN);
    
    if (election.area_count >= MAX_C) {
        show_msg("Maximum areas reached!", 2);
        wait_key();
        return;
    }
    
    printf("\n   Enter Area Name: ");
    if (get_string("", name_buf, 50) == 0) {
        show_msg("Name cannot be empty!", 2);
        wait_key();
        return;
    }
    
    for (i = 0; i < election.area_count; i++) {
        if (strcmp(area[i].name, name_buf) == 0) {
            show_msg("Area already exists!", 2);
            wait_key();
            return;
        }
    }
    
    printf("\n   Add '%s%s%s' as new area? (y/n): ", BGRN, name_buf, RST);
    cf = getch_custom();
    printf("%c\n", cf);
    
    if (cf != 'y' && cf != 'Y') {
        show_msg("Cancelled.", 0);
        wait_key();
        return;
    }
    
    area[election.area_count].id = election.area_count + 1;
    strncpy(area[election.area_count].name, name_buf, 49);
    area[election.area_count].name[49] = '\0';
    area[election.area_count].center_count = 0;
    area[election.area_count].voter_count = 0;
    for (i = 0; i < MAX_S; i++) area[election.area_count].centers[i] = 0;
    election.area_count++;
    
    progress("Adding area", 400);
    save_data();
    add_audit_log("New area added", "ADMIN");
    sound_coin();
    show_msg("Area added successfully!", 1);
    wait_key();
}

void view_areas() {
    int i;
    clear_screen();
    sound_whoosh();
    show_header("ALL VOTING AREAS", BCYN);
    
    if (election.area_count == 0) {
        printf("\n   %s⚠ No areas have been added yet.%s\n", YLW, RST);
        wait_key();
        return;
    }
    
    printf("\n");
    printf("   %s╔══════╦═════════════════════════╦══════════╦══════════╗%s\n", CYN, RST);
    printf("   %s║%s %sID%s   %s║%s %sAREA NAME%s               %s║%s %sCENTERS%s  %s║%s %sVOTERS%s   %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠══════╬═════════════════════════╬══════════╬══════════╣%s\n", CYN, RST);
    
    for (i = 0; i < election.area_count; i++) {
        printf("   %s║%s  %s%d%s   %s║%s %-23s %s║%s    %s%2d%s    %s║%s    %s%3d%s   %s║%s\n",
            CYN, RST, BGRN, area[i].id, RST, CYN, RST,
            area[i].name, CYN, RST,
            BYLW, area[i].center_count, RST, CYN, RST,
            BCYN, area[i].voter_count, RST, CYN, RST);
        printf("   %s╠──────┼─────────────────────────┼──────────┼──────────╣%s\n", CYN, RST);
        sound_pop();
        delay_ms(40);
    }
    printf("   %s╚══════╩═════════════════════════╩══════════╩══════════╝%s\n", CYN, RST);
    printf("\n   %sTotal:%s %s%d%s areas\n", YLW, RST, BGRN, election.area_count, RST);
    wait_key();
}

void manage_areas() {
    int choice;
    do {
        choice = area_menu();
        if (choice == 1) add_area();
        else if (choice == 2) view_areas();
    } while (choice != 3 && choice != 0);
}

int center_menu() {
    int choice;
    clear_screen();
    sound_whoosh();
    show_header("CENTER MANAGEMENT", BCYN);
    
    printf("\n   %sTotal Centers:%s %s%d%s / %d\n", YLW, RST, BGRN, election.center_count, RST, MAX_S);
    printf("\n");
    
    printf("   %s┌──────────────────────────────────────┐%s\n", GRN, RST);
    printf("   %s│%s  %s[1]%s  ➕ Add New Center              %s│%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s│%s  %s[2]%s  📋 View All Centers            %s│%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s│%s  %s[3]%s  🔙 Back                        %s│%s\n", GRN, RST, YLW, RST, GRN, RST);
    printf("   %s└──────────────────────────────────────┘%s\n", GRN, RST);
    
    printf("\n   %s▶%s Choice: ", BGRN, RST);
    if (get_int("", &choice, 1, 3) == 0) return 0;
    sound_click();
    return choice;
}

void add_center() {
    char name_buf[50];
    int area_id, capacity;
    int i, cid;
    char cf;
    
    clear_screen();
    sound_whoosh();
    show_header("ADD NEW VOTING CENTER", BGRN);
    
    if (election.center_count >= MAX_S) {
        show_msg("Maximum centers reached!", 2);
        wait_key();
        return;
    }
    
    if (election.area_count == 0) {
        show_msg("Add an area first!", 2);
        wait_key();
        return;
    }
    
    printf("\n   %sAvailable Areas:%s\n", YLW, RST);
    printf("   %s┌──────┬─────────────────────────┐%s\n", CYN, RST);
    for (i = 0; i < election.area_count; i++) {
        printf("   %s│%s  %s%d%s   %s│%s %-23s %s│%s\n", 
            CYN, RST, BGRN, area[i].id, RST, CYN, RST, area[i].name, CYN, RST);
    }
    printf("   %s└──────┴─────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   Select Area ID: ");
    if (get_int("", &area_id, 1, election.area_count) == 0) {
        show_msg("Invalid area!", 2);
        wait_key();
        return;
    }
    
    printf("   Enter Center Name: ");
    if (get_string("", name_buf, 50) == 0) {
        show_msg("Name cannot be empty!", 2);
        wait_key();
        return;
    }
    
    printf("   Enter Capacity (max voters): ");
    if (get_int("", &capacity, 1, 10000) == 0) capacity = 500;
    
    printf("\n   Add '%s%s%s' in '%s%s%s'? (y/n): ", 
        BGRN, name_buf, RST, BCYN, area[area_id - 1].name, RST);
    cf = getch_custom();
    printf("%c\n", cf);
    
    if (cf != 'y' && cf != 'Y') {
        show_msg("Cancelled.", 0);
        wait_key();
        return;
    }
    
    center[election.center_count].id = election.center_count + 1;
    strncpy(center[election.center_count].name, name_buf, 49);
    center[election.center_count].name[49] = '\0';
    center[election.center_count].area_id = area_id;
    center[election.center_count].vote_count = 0;
    center[election.center_count].is_open = 0;
    center[election.center_count].capacity = capacity;
    
    cid = election.center_count + 1;
    area[area_id - 1].centers[area[area_id - 1].center_count] = cid;
    area[area_id - 1].center_count++;
    election.center_count++;
    
    progress("Adding center", 400);
    save_data();
    add_audit_log("New center added", "ADMIN");
    sound_coin();
    show_msg("Center added successfully!", 1);
    wait_key();
}

void view_centers() {
    int i;
    char area_name[12];
    
    clear_screen();
    sound_whoosh();
    show_header("ALL VOTING CENTERS", BCYN);
    
    if (election.center_count == 0) {
        printf("\n   %s⚠ No centers have been added yet.%s\n", YLW, RST);
        wait_key();
        return;
    }
    
    printf("\n");
    printf("   %s╔══════╦═══════════════════╦═════════════╦══════════╦══════════╗%s\n", CYN, RST);
    printf("   %s║%s %sID%s   %s║%s %sCENTER NAME%s       %s║%s %sAREA%s        %s║%s %sCAPACITY%s %s║%s %sSTATUS%s   %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠══════╬═══════════════════╬═════════════╬══════════╬══════════╣%s\n", CYN, RST);
    
    for (i = 0; i < election.center_count; i++) {
        strcpy(area_name, "N/A");
        if (center[i].area_id >= 1 && center[i].area_id <= election.area_count) {
            strncpy(area_name, area[center[i].area_id - 1].name, 11);
            area_name[11] = '\0';
        }
        printf("   %s║%s  %s%d%s   %s║%s %-17s %s║%s %-11s %s║%s   %s%4d%s   %s║%s ",
            CYN, RST, BGRN, center[i].id, RST, CYN, RST,
            center[i].name, CYN, RST,
            area_name, CYN, RST,
            BCYN, center[i].capacity, RST, CYN, RST);
        if (center[i].is_open) printf("%s%-8s%s %s║%s\n", BGRN, "OPEN", RST, CYN, RST);
        else printf("%s%-8s%s %s║%s\n", BRED, "CLOSED", RST, CYN, RST);
        printf("   %s╠──────┼───────────────────┼─────────────┼──────────┼──────────╣%s\n", CYN, RST);
        sound_pop();
        delay_ms(100);
    }
    printf("   %s╚══════╩═══════════════════╩═════════════╩══════════╩══════════╝%s\n", CYN, RST);
    printf("\n   %sTotal:%s %s%d%s centers\n", YLW, RST, BGRN, election.center_count, RST);
    wait_key();
}

void manage_centers() {
    int choice;
    do {
        choice = center_menu();
        if (choice == 1) add_center();
        else if (choice == 2) view_centers();
    } while (choice != 3 && choice != 0);
}

void view_voters() {
    int i, voted_count = 0;
    char name_short[18];
    
    clear_screen();
    sound_whoosh();
    show_header("REGISTERED VOTERS", BCYN);
    
    if (election.voter_count == 0) {
        printf("\n   %s⚠ No voters registered yet.%s\n", YLW, RST);
        wait_key();
        return;
    }
    
    for (i = 0; i < election.voter_count; i++) {
        if (voter[i].voted) voted_count++;
    }
    
    printf("\n");
    printf("   %s┌────────────────────────────────────────────────────┐%s\n", CYN, RST);
    printf("   %s│%s  📊 Total: %s%d%s │ ✓ Voted: %s%d%s │ ○ Pending: %s%d%s      %s│%s\n",
        CYN, RST, BCYN, election.voter_count, RST,
        BGRN, voted_count, RST,
        BYLW, election.voter_count - voted_count, RST, CYN, RST);
    printf("   %s└────────────────────────────────────────────────────┘%s\n", CYN, RST);
    printf("\n");
    
    printf("   %s╔════════════════╦═══════════════════╦═════╦════════╗%s\n", CYN, RST);
    printf("   %s║%s %sNID%s            %s║%s %sNAME%s              %s║%s %sAGE%s %s║%s %sVOTED%s  %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠════════════════╬═══════════════════╬═════╬════════╣%s\n", CYN, RST);
    
    for (i = 0; i < election.voter_count; i++) {
        strncpy(name_short, voter[i].name, 17);
        name_short[17] = '\0';
        printf("   %s║%s %-14s %s║%s %-17s %s║%s %3d %s║%s ",
            CYN, RST, voter[i].nid, CYN, RST,
            name_short, CYN, RST,
            voter[i].age, CYN, RST);
        if (voter[i].voted) printf("%s%-6s%s %s║%s\n", BGRN, "YES", RST, CYN, RST);
        else printf("%s%-6s%s %s║%s\n", BYLW, "NO", RST, CYN, RST);
        printf("   %s╠────────────────┼───────────────────┼─────┼────────╣%s\n", CYN, RST);
        delay_ms(30);
    }
    printf("   %s╚════════════════╩═══════════════════╩═════╩════════╝%s\n", CYN, RST);
    wait_key();
}

void election_control() {
    int choice, i;
    char confirm[10];
    
    clear_screen();
    sound_whoosh();
    show_header("ELECTION CONTROL CENTER", BCYN);
    
    printf("\n   %sCurrent Status:%s ", YLW, RST);
    if (election.is_running) printf("%s● ELECTION IS LIVE%s\n", BGRN, RST);
    else printf("%s● ELECTION IS STOPPED%s\n", BRED, RST);
    
    printf("\n");
    printf("   %s┌──────────────────────────────────────┐%s\n", CYN, RST);
    printf("   %s│%s  %s[1]%s  ▶️  START Election              %s│%s\n", CYN, RST, BGRN, RST, CYN, RST);
    printf("   %s│%s  %s[2]%s  ⏹️  STOP Election               %s│%s\n", CYN, RST, BRED, RST, CYN, RST);
    printf("   %s│%s  %s[3]%s  🔙 Back                        %s│%s\n", CYN, RST, YLW, RST, CYN, RST);
    printf("   %s└──────────────────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   %s▶%s Choice: ", BGRN, RST);
    if (get_int("", &choice, 1, 3) == 0) return;
    
    if (choice == 1) {
        if (election.is_running) {
            show_msg("Election already running!", 3);
            wait_key();
            return;
        }
        if (election.area_count == 0 || election.center_count == 0) {
            show_msg("Setup areas and centers first!", 2);
            wait_key();
            return;
        }
        
        printf("\n   %sStarting Election in:%s ", CYN, RST);
        for (i = 3; i > 0; i--) {
            printf("%s%d%s ", BYLW, i, RST);
            fflush(stdout);
            sound_click();
            delay_ms(800);
        }
        printf("%sGO!%s\n", BGRN, RST);
        sound_bell();
        
        election.is_running = 1;
        for (i = 0; i < election.center_count; i++) center[i].is_open = 1;
        save_data();
        add_audit_log("Election STARTED", "ADMIN");
        sound_tada();
        show_msg("Election has STARTED!", 1);
        wait_key();
        
    } else if (choice == 2) {
        if (election.is_running == 0) {
            show_msg("Election already stopped!", 3);
            wait_key();
            return;
        }
        
        printf("\n   Type '%sSTOP%s' to confirm: ", BRED, RST);
        if (get_string("", confirm, 10) == 0 || strcmp(confirm, "STOP") != 0) {
            show_msg("Cancelled.", 0);
            wait_key();
            return;
        }
        
        progress("Stopping election", 800);
        election.is_running = 0;
        for (i = 0; i < election.center_count; i++) center[i].is_open = 0;
        save_data();
        add_audit_log("Election STOPPED", "ADMIN");
        show_msg("Election has STOPPED!", 1);
        wait_key();
    }
}

void view_audit_logs() {
    int i;
    clear_screen();
    sound_whoosh();
    show_header("AUDIT LOG VIEWER", BCYN);
    
    if (audit_count == 0) {
        printf("\n   %s⚠ No audit logs available.%s\n", YLW, RST);
        wait_key();
        return;
    }
    
    printf("\n");
    printf("   %s╔═══════════════════════╦══════════════════════════════════╦════════════╗%s\n", CYN, RST);
    printf("   %s║%s %sTIMESTAMP%s             %s║%s %sACTION%s                           %s║%s %sUSER%s       %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠═══════════════════════╬══════════════════════════════════╬════════════╣%s\n", CYN, RST);
    
    for (i = audit_count - 1; i >= 0 && i >= audit_count - 20; i--) {
        char action_short[33];
        strncpy(action_short, audit_log[i].action, 32);
        action_short[32] = '\0';
        
        printf("   %s║%s %-21s %s║%s %-32s %s║%s %-10s %s║%s\n",
            CYN, RST, audit_log[i].timestamp, CYN, RST,
            action_short, CYN, RST,
            audit_log[i].user, CYN, RST);
        printf("   %s╠───────────────────────┼──────────────────────────────────┼────────────╣%s\n", CYN, RST);
    }
    printf("   %s╚═══════════════════════╩══════════════════════════════════╩════════════╝%s\n", CYN, RST);
    printf("\n   %sShowing last 20 entries of %d total%s\n", DIM, audit_count, RST);
    wait_key();
}

void reset_data() {
    char confirm[20];
    
    clear_screen();
    sound_whoosh();
    animate_panel_entrance(BRED, 60);
    
    printf("%s\n", BRED);
    print_line_animated("   ╔═══════════════════════════════════════════════════════╗\n", 5);
    printf("   ║                                                       ║\n");
    printf("   ║        ⚠️  DANGER ZONE - DATA RESET ⚠️                 ║\n");
    printf("   ║                                                       ║\n");
    printf("   ╠═══════════════════════════════════════════════════════╣\n");
    printf("   ║                                                       ║\n");
    printf("   ║   This action will PERMANENTLY DELETE:                ║\n");
    printf("   ║                                                       ║\n");
    printf("   ║   • All registered voters: %-4d                       ║\n", election.voter_count);
    printf("   ║   • All voting areas: %-4d                            ║\n", election.area_count);
    printf("   ║   • All voting centers: %-4d                          ║\n", election.center_count);
    printf("   ║   • All vote counts                                   ║\n");
    printf("   ║   • All audit logs                                    ║\n");
    printf("   ║                                                       ║\n");
    printf("   ║   THIS CANNOT BE UNDONE!                              ║\n");
    printf("   ║                                                       ║\n");
    print_line_animated("   ╚═══════════════════════════════════════════════════════╝\n", 5);
    printf("%s\n", RST);
    
    printf("   Type '%sDELETE ALL%s' to confirm: ", BRED, RST);
    if (get_string("", confirm, 20) == 0 || strcmp(confirm, "DELETE ALL") != 0) {
        show_msg("Operation cancelled.", 0);
        wait_key();
        return;
    }
    
    progress("Deleting all data", 1200);
    clear_all_data();
    remove("election_data.dat");
    remove("security.dat");
    audit_count = 0;
    load_parties();
    save_data();
    add_audit_log("ALL DATA RESET", "ADMIN");
    show_msg("All data has been deleted!", 1);
    wait_key();
}

int voter_menu() {
    int choice;
    clear_screen();
    show_vote_banner();
    printf("\n");
    
    printf("   %s╔═════════════════════════[ VOTER SECTION ]════════════════════════╗%s\n", GRN, RST);
    printf("   %s║%s                                                                  %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s║%s   %s[1]%s  📝  New Voter Registration                                %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s   %s[2]%s  🔍  Check My Registration Status                          %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s   %s[3]%s  🔙  Back to Main Menu                                     %s║%s\n", GRN, RST, YLW, RST, GRN, RST);
    printf("   %s║%s                                                                  %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════════╝%s\n", GRN, RST);
    
    printf("\n   %s▶%s Choice: ", BGRN, RST);
    if (get_int("", &choice, 1, 3) == 0) return 0;
    sound_click();
    return choice;
}

int check_nid(char *nid) {
    int i, len = strlen(nid);
    if (len != NID_SIZE) return 0;
    for (i = 0; i < len; i++) {
        if (nid[i] < '0' || nid[i] > '9') return 0;
    }
    return 1;
}

int nid_exists(char *nid) {
    int i;
    for (i = 0; i < election.voter_count; i++) {
        if (strcmp(voter[i].nid, nid) == 0) return 1;
    }
    return 0;
}

int find_voter(char *nid) {
    int i;
    for (i = 0; i < election.voter_count; i++) {
        if (strcmp(voter[i].nid, nid) == 0) return i;
    }
    return -1;
}

int register_voter() {
    char nid_buf[20], name_buf[50], gender_buf[10], phone_buf[15];
    int age_val, area_val, center_val, i, valid, cid;
    
    clear_screen();
    sound_whoosh();
    show_header("VOTER REGISTRATION", BGRN);
    
    if (election.voter_count >= MAX_V) {
        show_msg("Registration limit reached!", 2);
        wait_key();
        return 0;
    }
    
    if (election.area_count == 0 || election.center_count == 0) {
        show_msg("System not configured yet!", 2);
        wait_key();
        return 0;
    }
    
    printf("\n   %s┌─────────────────────────────────────────────────────┐%s\n", CYN, RST);
    printf("   %s│%s           Enter Your Personal Information           %s│%s\n", CYN, RST, CYN, RST);
    printf("   %s└─────────────────────────────────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   📋 NID Number (13 digits): ");
    if (get_string("", nid_buf, 20) == 0) {
        show_msg("NID cannot be empty!", 2);
        wait_key();
        return 0;
    }
    
    loading("Validating NID", 400);
    
    if (check_nid(nid_buf) == 0) {
        show_msg("Invalid NID format!", 2);
        wait_key();
        return 0;
    }
    
    if (nid_exists(nid_buf)) {
        show_msg("NID already registered!", 2);
        wait_key();
        return 0;
    }
    
    printf("   👤 Full Name: ");
    if (get_string("", name_buf, 50) == 0) {
        show_msg("Name cannot be empty!", 2);
        wait_key();
        return 0;
    }
    
    printf("   🎂 Age: ");
    if (get_int("", &age_val, 1, 150) == 0) {
        show_msg("Invalid age!", 2);
        wait_key();
        return 0;
    }
    
    if (age_val < 18) {
        show_msg("Must be 18 or older to vote!", 2);
        wait_key();
        return 0;
    }
    
    printf("   ⚧ Gender (M/F/O): ");
    if (get_string("", gender_buf, 10) == 0) strcpy(gender_buf, "N/A");
    
    printf("   📱 Phone Number: ");
    if (get_string("", phone_buf, 15) == 0) strcpy(phone_buf, "N/A");
    
    printf("\n   %sSelect Your Voting Area:%s\n", YLW, RST);
    printf("   %s┌──────┬─────────────────────────┐%s\n", CYN, RST);
    for (i = 0; i < election.area_count; i++) {
        printf("   %s│%s  %s%d%s   %s│%s %-23s %s│%s\n", 
            CYN, RST, BGRN, area[i].id, RST, CYN, RST, area[i].name, CYN, RST);
    }
    printf("   %s└──────┴─────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   Area ID: ");
    if (get_int("", &area_val, 1, election.area_count) == 0) {
        show_msg("Invalid area!", 2);
        wait_key();
        return 0;
    }
    
    if (area[area_val - 1].center_count == 0) {
        show_msg("No centers in this area!", 2);
        wait_key();
        return 0;
    }
    
    printf("\n   %sCenters in %s:%s\n", YLW, area[area_val - 1].name, RST);
    printf("   %s┌──────┬─────────────────────────┐%s\n", CYN, RST);
    for (i = 0; i < area[area_val - 1].center_count; i++) {
        cid = area[area_val - 1].centers[i];
        if (cid > 0 && cid <= election.center_count) {
            printf("   %s│%s  %s%d%s   %s│%s %-23s %s│%s\n", 
                CYN, RST, BGRN, cid, RST, CYN, RST, center[cid - 1].name, CYN, RST);
        }
    }
    printf("   %s└──────┴─────────────────────────┘%s\n", CYN, RST);
    
    printf("\n   Center ID: ");
    if (get_int("", &center_val, 1, election.center_count) == 0) {
        show_msg("Invalid center!", 2);
        wait_key();
        return 0;
    }
    
    valid = 0;
    for (i = 0; i < area[area_val - 1].center_count; i++) {
        if (area[area_val - 1].centers[i] == center_val) {
            valid = 1;
            break;
        }
    }
    
    if (valid == 0) {
        show_msg("Center not in selected area!", 2);
        wait_key();
        return 0;
    }
    
    strncpy(voter[election.voter_count].nid, nid_buf, 13);
    voter[election.voter_count].nid[13] = '\0';
    strncpy(voter[election.voter_count].name, name_buf, 49);
    voter[election.voter_count].name[49] = '\0';
    voter[election.voter_count].age = age_val;
    voter[election.voter_count].area_id = area_val;
    voter[election.voter_count].center_id = center_val;
    voter[election.voter_count].voted = 0;
    strncpy(voter[election.voter_count].gender, gender_buf, 9);
    voter[election.voter_count].gender[9] = '\0';
    strncpy(voter[election.voter_count].phone, phone_buf, 14);
    voter[election.voter_count].phone[14] = '\0';
    get_today(voter[election.voter_count].date);
    election.voter_count++;
    area[area_val - 1].voter_count++;
    
    progress("Registering voter", 500);
    save_data();
    add_audit_log("New voter registered", nid_buf);
    sound_coin();
    
    printf("\n");
    printf("   %s╔═══════════════[ REGISTRATION COMPLETE ]═══════════════╗%s\n", BGRN, RST);
        printf("\n");
    printf("   %s╔═══════════════[ REGISTRATION COMPLETE ]═══════════════╗%s\n", BGRN, RST);
    printf("   %s║%s                                                       %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s║%s  NID     : %-42s %s║%s\n", GRN, RST, nid_buf, GRN, RST);
    printf("   %s║%s  Name    : %-42s %s║%s\n", GRN, RST, name_buf, GRN, RST);
    printf("   %s║%s  Center  : %-42s %s║%s\n", GRN, RST, center[center_val - 1].name, GRN, RST);
    printf("   %s║%s                                                       %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s║%s  %s✓ You are now eligible to vote!%s                      %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s                                                       %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚═══════════════════════════════════════════════════════╝%s\n", BGRN, RST);
    
    show_msg("Registration successful!", 1);
    wait_key();
    return 1;
}

void check_status() {
    char nid_buf[20];
    int idx;
    
    clear_screen();
    sound_whoosh();
    show_header("CHECK REGISTRATION STATUS", BCYN);
    
    printf("\n   Enter your NID: ");
    if (get_string("", nid_buf, 20) == 0) {
        show_msg("NID cannot be empty!", 2);
        wait_key();
        return;
    }
    
    loading("Searching database", 400);
    
    idx = find_voter(nid_buf);
    if (idx == -1) {
        show_msg("NID not found!", 2);
        wait_key();
        return;
    }
    
    printf("\n");
    printf("   %s╔═══════════════[ YOUR VOTER PROFILE ]═══════════════╗%s\n", BGRN, RST);
    printf("   %s║%s                                                    %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s║%s  👤 Name    : %-35s  %s║%s\n", GRN, RST, voter[idx].name, GRN, RST);
    printf("   %s║%s  🆔 NID     : %-35s  %s║%s\n", GRN, RST, voter[idx].nid, GRN, RST);
    printf("   %s║%s  🎂 Age     : %-35d  %s║%s\n", GRN, RST, voter[idx].age, GRN, RST);
    printf("   %s║%s  ⚧ Gender  : %-35s   %s║%s\n", GRN, RST, voter[idx].gender, GRN, RST);
    printf("   %s║%s  📱 Phone   : %-35s  %s║%s\n", GRN, RST, voter[idx].phone, GRN, RST);
    printf("   %s║%s  🗺️  Area    : %-35s  %s║%s\n", GRN, RST, area[voter[idx].area_id - 1].name, GRN, RST);
    printf("   %s║%s  🏢 Center  : %-35s  %s║%s\n", GRN, RST, center[voter[idx].center_id - 1].name, GRN, RST);
    printf("   %s║%s  📅 Reg.Date: %-35s  %s║%s\n", GRN, RST, voter[idx].date, GRN, RST);
    printf("   %s║%s                                                    %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s║%s  🗳️  Vote Status: ", GRN, RST);
    if (voter[idx].voted) printf("%s✓ VOTED%s                             %s║%s\n", BGRN, RST, GRN, RST);
    else printf("%s○ NOT YET VOTED%s                   %s║%s\n", BYLW, RST, GRN, RST);
    printf("   %s║%s                                                    %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚════════════════════════════════════════════════════╝%s\n", BGRN, RST);
    
    sound_success();
    wait_key();
}

void enter_booth() {
    int i, j, k;
    
    for (i = 15; i >= 0; i--) {
        clear_screen();
        printf("\n\n");
        for (j = 0; j < 6; j++) {
            printf("   ");
            for (k = 0; k < i; k++) printf("%s█%s", DIM, RST);
            for (k = 0; k < (30 - 2 * i); k++) printf(" ");
            for (k = 0; k < i; k++) printf("%s█%s", DIM, RST);
            printf("\n");
        }
        fflush(stdout);
        delay_ms(20);
    }
    
    printf("%s\n", WHT);
    printf("   ╔════════════════════════════════════════╗\n");
    printf("   ║                                        ║\n");
    printf("   ║     🗳️  ENTERING VOTING BOOTH 🗳️       ║\n");
    printf("   ║                                        ║\n");
    printf("   ║       Your vote is CONFIDENTIAL        ║\n");
    printf("   ║                                        ║\n");
    printf("   ╚════════════════════════════════════════╝\n");
    printf("%s", RST);
    sound_success();
    delay_ms(1000);
}

void show_ballot() {
    clear_screen();
    sound_ding();
    printf("%s\n", WHT);
    print_line_animated("   ╔════════════════════════════════════════════════════════════╗\n", 5);
    printf("   ║                                                            ║\n");
    printf("   ║                  🗳️  OFFICIAL BALLOT 🗳️                    ║\n");
    printf("   ║                                                            ║\n");
    printf("   ╠════════════════════════════════════════════════════════════╣\n");
    printf("   ║                                                            ║\n");
    printf("   ║   Election : %-44s ║\n", election.name);
    printf("   ║   Date     : %-44s ║\n", election.date);
    printf("   ║                                                            ║\n");
    printf("   ╠════════════════════════════════════════════════════════════╣\n");
    printf("   ║                                                            ║\n");
    printf("   ║          ⚠️  SELECT ONLY ONE (1) PARTY ⚠️                   ║\n");
    printf("   ║                                                            ║\n");
    print_line_animated("   ╚════════════════════════════════════════════════════════════╝\n", 5);
    printf("%s", RST);
}

int cast_vote(char *nid) {
    int i, idx, center_input, choice;
    char confirm;
    char *sym_names[] = {"", "Rice", "Lily", "Scales", "Pen", "Hand Fan"};
    char *colors[] = {"", GRN, CYN, YLW, MAG, RED};
    
    idx = find_voter(nid);
    if (idx == -1) {
        show_msg("Voter not found!", 2);
        wait_key();
        return 0;
    }
    
    if (voter[idx].voted) {
        show_msg("You have already voted!", 2);
        wait_key();
        return 0;
    }
    
    if (election.is_running == 0) {
        show_msg("Election is not running!", 2);
        wait_key();
        return 0;
    }
    
    if (center[voter[idx].center_id - 1].is_open == 0) {
        show_msg("Your voting center is closed!", 2);
        wait_key();
        return 0;
    }
    
    printf("\n   %s📍 Your assigned center:%s %s%s%s\n", 
        YLW, RST, BGRN, center[voter[idx].center_id - 1].name, RST);
    printf("   Enter your current center ID: ");
    
    if (get_int("", &center_input, 1, election.center_count) == 0) {
        show_msg("Invalid input!", 2);
        wait_key();
        return 0;
    }
    
    loading("Verifying location", 500);
    
    if (center_input != voter[idx].center_id) {
        show_msg("Location mismatch!", 2);
        printf("\n   %s⚠ Please go to:%s %s%s%s\n", YLW, RST, BGRN, center[voter[idx].center_id - 1].name, RST);
        wait_key();
        return 0;
    }
    
    enter_booth();
    show_ballot();
    
    printf("\n   %s╔═════════════════[ SELECT YOUR PARTY ]═════════════════╗%s\n", BYLW, RST);
    printf("   %s║%s                                                       %s║%s\n", BYLW, RST, BYLW, RST);
    for (i = 0; i < election.party_count; i++) {
        printf("   %s║%s   %s[%d]%s  %s%-15s%s  │  Symbol: %-10s    %s║%s\n", 
            BYLW, RST, colors[i+1], i + 1, RST, 
            colors[i+1], party[i].name, RST,
            sym_names[party[i].symbol], BYLW, RST);
    }
    printf("   %s║%s                                                       %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s╚═══════════════════════════════════════════════════════╝%s\n", BYLW, RST);
    
    printf("\n   Show party symbols? (y/n): ");
    confirm = getch_custom();
    printf("%c\n", confirm);
    
    if (confirm == 'y' || confirm == 'Y') {
        for (i = 0; i < election.party_count; i++) {
            printf("\n   %s[%d] %s%s%s\n", CYN, i + 1, colors[i+1], party[i].name, RST);
            show_symbol_animated(party[i].symbol);
            delay_ms(200);
        }
    }
    
    printf("\n   %s▶%s Enter your choice (1-%d): ", BGRN, RST, election.party_count);
    if (get_int("", &choice, 1, election.party_count) == 0) {
        show_msg("Invalid choice!", 2);
        wait_key();
        return 0;
    }
    
    printf("\n   %s┌─────────────────────────────────────────────────────┐%s\n", YLW, RST);
    printf("   %s│%s  You selected: %s%-35s%s %s│%s\n", YLW, RST, colors[choice], party[choice - 1].name, RST, YLW, RST);
    printf("   %s│%s  Symbol: %-43s %s│%s\n", YLW, RST, sym_names[party[choice - 1].symbol], YLW, RST);
    printf("   %s└─────────────────────────────────────────────────────┘%s\n", YLW, RST);
    
    printf("\n   %s⚠ Confirm your vote? (y/n):%s ", BYLW, RST);
    confirm = getch_custom();
    printf("%c\n", confirm);
    
    if (confirm != 'y' && confirm != 'Y') {
        show_msg("Vote cancelled.", 0);
        wait_key();
        return 0;
    }
    
    voter[idx].voted = 1;
    party[choice - 1].total++;
    center[voter[idx].center_id - 1].vote_count++;
    save_data();
    add_audit_log("Vote cast successfully", nid);
    
    return 1;
}

void ballot_drop() {
    int row, i, f;
    char *ballot[] = {
        "   ╔═══════════╗",
        "   ║  [X] VOTE ║",
        "   ╚═══════════╝"
    };
    
    for (row = 1; row <= 12; row++) {
        clear_screen();
        for (i = 0; i < row; i++) printf("\n");
        printf("%s", BCYN);
        for (i = 0; i < 3; i++) printf("%s\n", ballot[i]);
        printf("%s", RST);
        fflush(stdout);
        delay_ms(40);
    }
    
    sound_vote();
    
    for (f = 0; f < 6; f++) {
        clear_screen();
        for (i = 0; i < 10; i++) printf("\n");
        if (f % 2 == 0) printf("%s", BGRN);
        else printf("%s", BYLW);
        printf("      ╔═══════════════════════════════╗\n");
        printf("      ║     ✓ VOTE RECORDED ✓        ║\n");
        printf("      ╚═══════════════════════════════╝\n");
        printf("%s", RST);
        fflush(stdout);
        delay_ms(80);
    }
    delay_ms(300);
}

void vote_done() {
    printf("\n");
    show_msg("Your vote has been recorded!", 1);
    sound_tada();
    printf("\n");
    
    printf("   %s╔═══════════════════════════════════════════════════════╗%s\n", BGRN, RST);
    printf("   %s║%s                                                       %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s║%s        🎉 THANK YOU FOR VOTING! 🎉                    %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s║%s                                                       %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s║%s        Your vote is SECRET and SECURE                 %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s║%s        Democracy depends on YOU!                      %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s║%s                                                       %s║%s\n", BGRN, RST, BGRN, RST);
    printf("   %s╚═══════════════════════════════════════════════════════╝%s\n", BGRN, RST);
    
    wait_key();
}

void live_statistics() {
    int i, total_votes = 0, voted_count = 0;
    float turnout;
    char time_buf[10], date_buf[15];
    
    clear_screen();
    sound_whoosh();
    get_today(date_buf);
    get_current_time(time_buf);
    
    for (i = 0; i < election.party_count; i++) total_votes += party[i].total;
    for (i = 0; i < election.voter_count; i++) if (voter[i].voted) voted_count++;
    turnout = election.voter_count > 0 ? (float)voted_count / election.voter_count * 100 : 0;
    
    animate_panel_entrance(BCYN, 70);
    
    printf("%s\n", BCYN);
    print_line_animated("   ╔═══════════════════════════════════════════════════════════════════╗\n", 3);
    printf("   ║                                                                   ║\n");
    printf("   ║            📊 LIVE ELECTION STATISTICS DASHBOARD 📊               ║\n");
    printf("   ║                                                                   ║\n");
    print_line_animated("   ╚═══════════════════════════════════════════════════════════════════╝\n", 3);
    printf("%s\n", RST);
    
    printf("   %s╔═══════════════════════════════════════════════════════════════════╗%s\n", DIM, RST);
    printf("   %s║%s  📅 Date: %s  │  ⏰ Time: %s  │  Status: %s%s%s     %s║%s\n", 
        DIM, RST, date_buf, time_buf, 
        election.is_running ? BGRN : BRED,
        election.is_running ? "🟢 LIVE" : "🔴 OFF",
        RST, DIM, RST);
    printf("   %s╚═══════════════════════════════════════════════════════════════════╝%s\n", DIM, RST);
    printf("\n");
    
    printf("   %s╔═══════════════════════[ KEY METRICS ]════════════════════════╗%s\n", BYLW, RST);
    printf("   %s║%s                                                              %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s║%s   👥 Registered Voters  : %s%-30d%s     %s║%s\n", BYLW, RST, BCYN, election.voter_count, RST, BYLW, RST);
    printf("   %s║%s   🗳️  Total Votes Cast   : %s%-30d%s     %s║%s\n", BYLW, RST, BGRN, total_votes, RST, BYLW, RST);
    printf("   %s║%s   ⏳ Pending Voters     : %s%-30d%s     %s║%s\n", BYLW, RST, BYLW, election.voter_count - voted_count, RST, BYLW, RST);
    printf("   %s║%s   📈 Voter Turnout      : %s%-29.1f%%%s     %s║%s\n", BYLW, RST, BMAG, turnout, RST, BYLW, RST);
    printf("   %s║%s                                                              %s║%s\n", BYLW, RST, BYLW, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════╝%s\n", BYLW, RST);
    printf("\n");
    
    printf("   %s╔═══════════════════════[ AREA STATISTICS ]════════════════════╗%s\n", CYN, RST);
    printf("   %s║%s                                                              %s║%s\n", CYN, RST, CYN, RST);
    for (i = 0; i < election.area_count && i < 8; i++) {
        printf("   %s║%s   🗺️  %-20s : %s%3d%s voters registered           %s║%s\n", 
            CYN, RST, area[i].name, BGRN, area[i].voter_count, RST, CYN, RST);
    }
    if (election.area_count == 0) {
        printf("   %s║%s   No areas configured yet                                    %s║%s\n", CYN, RST, CYN, RST);
    }
    printf("   %s║%s                                                              %s║%s\n", CYN, RST, CYN, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════╝%s\n", CYN, RST);
    printf("\n");
    
    printf("   %s╔═══════════════════════[ CENTER STATUS ]══════════════════════╗%s\n", GRN, RST);
    printf("   %s║%s                                                              %s║%s\n", GRN, RST, GRN, RST);
    for (i = 0; i < election.center_count && i < 6; i++) {
        printf("   %s║%s   🏢 %-18s : %s%-6s%s │ Votes: %s%3d%s            %s║%s\n", 
            GRN, RST, center[i].name,
            center[i].is_open ? BGRN : BRED,
            center[i].is_open ? "OPEN" : "CLOSED", RST,
            BCYN, center[i].vote_count, RST, GRN, RST);
    }
    if (election.center_count == 0) {
        printf("   %s║%s   No centers configured yet                                  %s║%s\n", GRN, RST, GRN, RST);
    }
    printf("   %s║%s                                                              %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚══════════════════════════════════════════════════════════════╝%s\n", GRN, RST);
    
    wait_key();
}

void bar_chart() {
    int i, j, max_vote = 0, bar_len;
    char *colors[] = {BGRN, BCYN, BYLW, BMAG, BRED};
    
    clear_screen();
    sound_whoosh();
    animate_panel_entrance(BYLW, 70);
    
    printf("%s\n", BYLW);
    print_line_animated("   ╔═══════════════════════════════════════════════════════════════════╗\n", 3);
    printf("   ║                                                                   ║\n");
    printf("   ║                 📊 VOTE DISTRIBUTION CHART 📊                     ║\n");
    printf("   ║                                                                   ║\n");
    print_line_animated("   ╚═══════════════════════════════════════════════════════════════════╝\n", 3);
    printf("%s\n", RST);
    
    for (i = 0; i < election.party_count; i++) {
        if (party[i].total > max_vote) max_vote = party[i].total;
    }
    
    printf("   %s╔═══════════════╦════════════════════════════════════════╦═══════╗%s\n", CYN, RST);
    printf("   %s║%s %sPARTY%s         %s║%s %sBAR GRAPH%s                              %s║%s %sVOTES%s %s║%s\n", 
           CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST, WHT, RST, CYN, RST);
    printf("   %s╠═══════════════╬════════════════════════════════════════╬═══════╣%s\n", CYN, RST);
    
    for (i = 0; i < election.party_count; i++) {
        bar_len = 0;
        if (max_vote > 0) bar_len = (party[i].total * 30) / max_vote;
        
        printf("   %s║%s %s%-13s%s %s║%s ", CYN, RST, colors[i], party[i].name, RST, CYN, RST);
        
        for (j = 0; j <= bar_len; j++) {
            printf("%s█%s", colors[i], RST);
            fflush(stdout);
            delay_ms(15);
        }
        for (j = bar_len + 1; j <= 30; j++) printf(" ");
        
        printf(" %s║%s %s%5d%s %s║%s\n", CYN, RST, BGRN, party[i].total, RST, CYN, RST);
        printf("   %s╠═══════════════╬════════════════════════════════════════╬═══════╣%s\n", CYN, RST);
        sound_pop();
    }
    printf("   %s╚═══════════════╩════════════════════════════════════════╩═══════╝%s\n", CYN, RST);
    
    wait_key();
}

void national_result() {
    int i, j, total = 0, winner = 0, max_votes = 0, order[MAX_P], temp, id;
    float percent, turnout;
    char *sym_names[] = {"", "Rice", "Lily", "Scales", "Pen", "Hand Fan"};
    char *colors[] = {"", BGRN, BCYN, BYLW, BMAG, BRED};
    
    clear_screen();
    show_result_banner();
    sound_drum();
    
    printf("\n   %sElection:%s %s%s%s\n", YLW, RST, WHT, election.name, RST);
    printf("   %sDate:%s %s%s%s\n", YLW, RST, WHT, election.date, RST);
    
    loading("Calculating final results", 600);
    
    for (i = 0; i < election.party_count; i++) total += party[i].total;
    
    for (i = 0; i < election.party_count; i++) {
        if (party[i].total > max_votes) {
            max_votes = party[i].total;
            winner = i;
        }
    }
    
    for (i = 0; i < election.party_count; i++) order[i] = i;
    for (i = 0; i < election.party_count - 1; i++) {
        for (j = 0; j < election.party_count - 1 - i; j++) {
            if (party[order[j]].total < party[order[j + 1]].total) {
                temp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = temp;
            }
        }
    }
    
    printf("\n");
    printf("   %s╔══════╦════════════════╦════════════╦═════════╦═════════╦════════╗%s\n", BCYN, RST);
    printf("   %s║%s %sRANK%s %s║%s %sPARTY%s          %s║%s %sSYMBOL%s     %s║%s %sVOTES%s   %s║%s   %s%%     %s║%s %sSTATUS%s %s║%s\n", 
           BCYN, RST, WHT, RST, BCYN, RST, WHT, RST, BCYN, RST, WHT, RST, BCYN, RST, WHT, RST, BCYN, RST, WHT, RST, BCYN, RST, WHT, RST, BCYN, RST);
    printf("   %s╠══════╬════════════════╬════════════╬═════════╬═════════╬════════╣%s\n", BCYN, RST);
    
    for (i = 0; i < election.party_count; i++) {
        id = order[i];
        percent = 0;
        if (total > 0) percent = (float)party[id].total / total * 100;
        
        printf("   %s║%s  %s%d%s   %s║%s %s%-14s%s %s║%s %-10s %s║%s  %s%5d%s  %s║%s %s%5.1f%%%s  %s║%s",
            BCYN, RST,
            (id == winner && total > 0) ? BGRN : WHT, i + 1, RST,
            BCYN, RST,
            colors[id + 1], party[id].name, RST,
            BCYN, RST,
            sym_names[party[id].symbol],
            BCYN, RST,
            BYLW, party[id].total, RST,
            BCYN, RST,
            BMAG, percent, RST,
            BCYN, RST);
        
        if (id == winner && total > 0) printf(" %s🏆WIN%s ", BGRN, RST);
        else printf("       ");
        printf("%s║%s\n", BCYN, RST);
        
        printf("   %s╠──────┼────────────────┼────────────┼─────────┼─────────┼────────╣%s\n", BCYN, RST);
        sound_pop();
        delay_ms(600);
    }
    printf("   %s╚══════╩════════════════╩════════════╩═════════╩═════════╩════════╝%s\n", BCYN, RST);
    
    printf("\n");
    printf("   %s╔═════════════════════════[ ELECTION SUMMARY ]════════════════════════╗%s\n", CYN, RST);
    printf("   %s║%s                                                                     %s║%s\n", CYN, RST, CYN, RST);
    printf("   %s║%s   📊 Total Votes Cast    : %s%-35d%s      %s║%s\n", CYN, RST, BGRN, total, RST, CYN, RST);
    printf("   %s║%s   👥 Registered Voters   : %s%-35d%s      %s║%s\n", CYN, RST, BCYN, election.voter_count, RST, CYN, RST);
    if (election.voter_count > 0) {
        turnout = (float)total / election.voter_count * 100;
        printf("   %s║%s   📈 Voter Turnout       : %s%-34.1f%%%s      %s║%s\n", CYN, RST, BYLW, turnout, RST, CYN, RST);
    }
    printf("   %s║%s   🏢 Total Centers       : %s%-35d%s      %s║%s\n", CYN, RST, BMAG, election.center_count, RST, CYN, RST);
    printf("   %s║%s   🗺️  Total Areas         : %s%-35d%s      %s║%s\n", CYN, RST, BCYN, election.area_count, RST, CYN, RST);
    printf("   %s║%s                                                                     %s║%s\n", CYN, RST, CYN, RST);
    printf("   %s╚═════════════════════════════════════════════════════════════════════╝%s\n", CYN, RST);
    
    if (total > 0) {
        printf("\n");
        printf("   %s╔═════════════════════════════════════════════════════════════════════╗%s\n", BGRN, RST);
        printf("   %s║%s                                                                     %s║%s\n", BGRN, RST, BGRN, RST);
        printf("   %s║%s        🏆 ELECTION WINNER: %s%-35s%s    %s║%s\n", BGRN, RST, BYLW, party[winner].name, RST, BGRN, RST);
        printf("   %s║%s        📊 Winning Votes  : %s%-35d%s    %s║%s\n", BGRN, RST, BCYN, party[winner].total, RST, BGRN, RST);
        printf("   %s║%s                                                                     %s║%s\n", BGRN, RST, BGRN, RST);
        printf("   %s╚═════════════════════════════════════════════════════════════════════╝%s\n", BGRN, RST);
        sound_ding();
        
        printf("\n");
        printf("   %s┌──────────────────────────────────────────────────┐%s\n", YLW, RST);
        printf("   %s│%s  Press any key to see the WINNER TROPHY...      %s│%s\n", YLW, RST, YLW, RST);
        printf("   %s└──────────────────────────────────────────────────┘%s\n", YLW, RST);
        getch_custom();
        show_trophy_animated();
    } else {
        printf("\n   %s⚠ No votes have been cast yet.%s\n", BYLW, RST);
    }
    
    wait_key();
}

void result_menu() {
    int choice;
    
    do {
        clear_screen();
        show_result_banner();
        printf("\n");
        
        printf("   %s╔═════════════════════════[ RESULTS MENU ]═════════════════════════╗%s\n", BYLW, RST);
        printf("   %s║%s                                                                  %s║%s\n", BYLW, RST, BYLW, RST);
        printf("   %s║%s   %s[1]%s  🏆  View National Results                                 %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
        printf("   %s║%s   %s[2]%s  📊  View Bar Chart                                        %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
        printf("   %s║%s   %s[3]%s  🔙  Back to Main Menu                                     %s║%s\n", BYLW, RST, YLW, RST, BYLW, RST);
        printf("   %s║%s                                                                  %s║%s\n", BYLW, RST, BYLW, RST);
        printf("   %s╚══════════════════════════════════════════════════════════════════╝%s\n", BYLW, RST);
        
        printf("\n   %s▶%s Choice: ", BGRN, RST);
        if (get_int("", &choice, 1, 3) == 0) choice = 0;
        sound_click();
        
        if (choice == 1) national_result();
        else if (choice == 2) bar_chart();
    } while (choice != 3 && choice != 0);
}

void settings_menu() {
    int choice;
    
    do {
        clear_screen();
        sound_whoosh();
        animate_panel_entrance(BCYN, 70);
        
        printf("%s\n", BCYN);
        print_line_animated("   ╔═══════════════════════════════════════════════════════════════════╗\n", 3);
        printf("   ║                                                                   ║\n");
        printf("   ║                    ⚙️  SYSTEM SETTINGS ⚙️                          ║\n");
        printf("   ║                                                                   ║\n");
        print_line_animated("   ╚═══════════════════════════════════════════════════════════════════╝\n", 3);
        printf("%s\n", RST);
        
        printf("   %s╔═════════════════════[ Current Settings ]═════════════════════╗%s\n", CYN, RST);
        printf("   %s║%s                                                             %s║%s\n", CYN, RST, CYN, RST);
        printf("   %s║%s   🔊 Sound Effects    : %s%-33s%s %s║%s\n", CYN, RST, sound_enabled ? BGRN : BRED, sound_enabled ? "ENABLED" : "DISABLED", RST, CYN, RST);
        printf("   %s║%s   🎬 Animations       : %s%-33s%s %s║%s\n", CYN, RST, animation_enabled ? BGRN : BRED, animation_enabled ? "ENABLED" : "DISABLED", RST, CYN, RST);
        printf("   %s║%s                                                             %s║%s\n", CYN, RST, CYN, RST);
        printf("   %s╚═══════════════════════════════════════════════════════════════╝%s\n", CYN, RST);
        printf("\n");
        
        printf("   %s╔═══════════════════════════════════════════════════════════════════╗%s\n", BYLW, RST);
        printf("   %s║%s                                                                   %s║%s\n", BYLW, RST, BYLW, RST);
        printf("   %s║%s   %s[1]%s  🔊  Toggle Sound Effects                                  %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
        printf("   %s║%s   %s[2]%s  🎬  Toggle Animations                                     %s║%s\n", BYLW, RST, BGRN, RST, BYLW, RST);
        printf("   %s║%s   %s[3]%s  🔐  Change Admin Password                                 %s║%s\n", BYLW, RST, BCYN, RST, BYLW, RST);
        printf("   %s║%s   %s[4]%s  🔓  Reset Admin Lockout                                   %s║%s\n", BYLW, RST, BMAG, RST, BYLW, RST);
        printf("   %s║%s   %s[5]%s  🔙  Back to Main Menu                                     %s║%s\n", BYLW, RST, YLW, RST, BYLW, RST);
        printf("   %s║%s                                                                   %s║%s\n", BYLW, RST, BYLW, RST);
        printf("   %s╚═══════════════════════════════════════════════════════════════════╝%s\n", BYLW, RST);
        
        printf("\n   %s▶%s Choice: ", BGRN, RST);
        if (get_int("", &choice, 1, 5) == 0) choice = 0;
        sound_click();
        
        if (choice == 1) {
            sound_enabled = !sound_enabled;
            show_msg(sound_enabled ? "Sound ENABLED" : "Sound DISABLED", 1);
            delay_ms(500);
        } else if (choice == 2) {
            animation_enabled = !animation_enabled;
            show_msg(animation_enabled ? "Animations ENABLED" : "Animations DISABLED", 1);
            delay_ms(500);
        } else if (choice == 3) {
            char new_pass[30], confirm_pass[30];
            printf("\n   Enter new password: ");
            if (get_string("", new_pass, 30) && strlen(new_pass) >= 4) {
                printf("   Confirm new password: ");
                if (get_string("", confirm_pass, 30)) {
                    if (strcmp(new_pass, confirm_pass) == 0) {
                        strncpy(PASSWORD, new_pass, 29);
                        PASSWORD[29] = '\0';
                        add_audit_log("Admin password changed", "ADMIN");
                        show_msg("Password changed!", 1);
                    } else {
                        show_msg("Passwords don't match!", 2);
                    }
                }
            } else {
                show_msg("Password too short!", 2);
            }
            wait_key();
        } else if (choice == 4) {
            admin_security.failed_attempts = 0;
            admin_security.is_locked = 0;
            admin_security.lockout_time = 0;
            save_security_data();
            add_audit_log("Admin lockout reset", "SYSTEM");
            show_msg("Lockout reset!", 1);
            delay_ms(500);
        }
    } while (choice != 5 && choice != 0);
}

void show_exit_animation() {
    int i, j;
    char *goodbye[] = {
        "   ╔═══════════════════════════════════════════════════════════════════╗",
        "   ║                                                                   ║",
        "   ║       Thank you for using the Election Management System          ║",
        "   ║                                                                   ║",
        "   ║                      👋 Goodbye! 👋                               ║",
        "   ║                                                                   ║",
        "   ╚═══════════════════════════════════════════════════════════════════╝"
    };
    
    stop_all_sounds();
    clear_screen();
    printf("%s\n\n", BCYN);
    
    for (i = 0; i < 7; i++) {
        print_slow(goodbye[i], 4);
        printf("\n");
    }
    printf("%s", RST);
    
    printf("\n");
    printf("   %s╔═══════════════════════════════════════════════════════════════════╗%s\n", GRN, RST);
    printf("   %s║%s  Developed by: %sIIUC CSE 1CM Team%s                                  %s║%s\n", GRN, RST, BGRN, RST, GRN, RST);
    printf("   %s║%s  © 2026 All Rights Reserved                                       %s║%s\n", GRN, RST, GRN, RST);
    printf("   %s╚═══════════════════════════════════════════════════════════════════╝%s\n", GRN, RST);
    
    printf("\n   ");
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 3; j++) {
            printf("%s.%s", (i % 2 == 0) ? BYLW : BGRN, RST);
            fflush(stdout);
            delay_ms(100);
        }
    }
    
    play_sound(SND_SUCCESS);
    delay_ms(1500);
}

int main() {
    int choice, admin_choice, voter_choice;
    char nid_buf[20];
    
    #ifdef _WIN32
        system("color");
        system("chcp 65001 >nul");
    #endif
    
    srand(time(NULL));
    
    clear_all_data();
    load_data();
    load_security_data();
    
    show_splash_screen();
    
    while (1) {
        choice = main_menu();
        
        switch (choice) {
            case 1:
                if (admin_login()) {
                    do {
                        admin_choice = admin_menu();
                        switch (admin_choice) {
                            case 1: admin_setup(); break;
                            case 2: admin_parties(); break;
                            case 3: manage_areas(); break;
                            case 4: manage_centers(); break;
                            case 5: view_voters(); break;
                            case 6: election_control(); break;
                            case 7: view_audit_logs(); break;
                            case 8: reset_data(); break;
                            case 9:
                                play_sound(SND_CLICK);
                                add_audit_log("Admin logged out", "ADMIN");
                                show_msg("Logged out successfully!", 1);
                                delay_ms(800);
                                break;
                            default:
                                if (admin_choice != 0) {
                                    show_msg("Invalid choice!", 2);
                                    delay_ms(800);
                                }
                                break;
                        }
                    } while (admin_choice != 9 && admin_choice != 0);
                }
                break;
                
            case 2:
                do {
                    voter_choice = voter_menu();
                    switch (voter_choice) {
                        case 1: register_voter(); break;
                        case 2: check_status(); break;
                        case 3: break;
                        default:
                            if (voter_choice != 0) {
                                show_msg("Invalid choice!", 2);
                                delay_ms(800);
                            }
                            break;
                    }
                } while (voter_choice != 3 && voter_choice != 0);
                break;
                
            case 3:
                clear_screen();
                show_vote_banner();
                
                if (election.is_running == 0) {
                    show_msg("Election is not running!", 2);
                    wait_key();
                } else {
                    printf("\n");
                    printf("   %s╔═══════════════════════════════════════════════════════╗%s\n", CYN, RST);
                    printf("   %s║%s                                                       %s║%s\n", CYN, RST, CYN, RST);
                    printf("   %s║%s           🗳️  VOTER IDENTIFICATION 🗳️                 %s║%s\n", CYN, RST, CYN, RST);
                    printf("   %s║%s                                                       %s║%s\n", CYN, RST, CYN, RST);
                    printf("   %s╚═══════════════════════════════════════════════════════╝%s\n", CYN, RST);
                    
                    printf("\n   Enter your NID to vote: ");
                    if (get_string("", nid_buf, 20)) {
                        if (cast_vote(nid_buf)) {
                            ballot_drop();
                            vote_done();
                        }
                    } else {
                        show_msg("Invalid input!", 2);
                        wait_key();
                    }
                }
                break;
                
            case 4:
                result_menu();
                break;
                
            case 5:
                live_statistics();
                break;
                
            case 6:
                settings_menu();
                break;
                
            case 7:
                show_about();
                break;
                
            case 8:
                show_exit_animation();
                save_data();
                save_security_data();
                return 0;
                
            default:
                if (choice != 0) {
                    show_msg("Invalid choice! Try again.", 2);
                    delay_ms(800);
                }
                break;
        }
    }
    
    return 0;
}                                                       