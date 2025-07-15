#include <stddef.h>


// stdint_custom.h
#ifndef STDINT_CUSTOM_H
#define STDINT_CUSTOM_H

// Tipos inteiros com sinal
typedef signed char        int8_t;
typedef signed short       int16_t;
typedef signed int         int32_t;
typedef signed long long   int64_t;

// Tipos inteiros sem sinal
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

// Macros para limites (opcional, mas útil)
#define INT8_MIN    (-128)
#define INT8_MAX    127
#define UINT8_MAX   255

#define INT16_MIN   (-32768)
#define INT16_MAX   32767
#define UINT16_MAX  65535

#define INT32_MIN   (-2147483648)
#define INT32_MAX   2147483647
#define UINT32_MAX  4294967295U

#define INT64_MIN   (-9223372036854775807LL - 1)
#define INT64_MAX   9223372036854775807LL
#define UINT64_MAX  18446744073709551615ULL

#endif // STDINT_CUSTOM_H

#ifndef NULL
#define NULL ((void*)0)
#endif

void outb(uint16_t port,uint8_t data);
void scroll();

volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
uint8_t cursor_row = 0;
uint8_t cursor_col = 0;



int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, unsigned int n) {
    while (n && *s1 && (*s1 == *s2)) {
        ++s1;
        ++s2;
        --n;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;


// Atualiza o cursor de hardware da VGA (o underline piscante)
void update_cursor() {
    uint16_t pos = cursor_row * 80 + cursor_col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Escreve um caractere na tela com cor padrão
void printp(const char* str) {
    while (*str) {
        char c = *str++;

        if (c == '\n') {
            cursor_row++;
            cursor_col = 0;
        } else {
            if (cursor_row >= 25) scroll();
            uint16_t pos = cursor_row * 80 + cursor_col;
            vga_buffer[pos] = ((uint8_t)0x0F << 8) | c;
            cursor_col++;
            if (cursor_col >= 80) {
                cursor_col = 0;
                cursor_row++;
            }
        }
    }
    update_cursor(); // atualiza posição do cursor na tela
}

// Scroll básico da tela para cima
void scroll() {
    for (int i = 0; i < 24 * 80; i++) {
        vga_buffer[i] = vga_buffer[i + 80];
    }
    for (int i = 24 * 80; i < 25 * 80; i++) {
        vga_buffer[i] = (0x0F << 8) | ' ';
    }
    cursor_row = 24;
}

// Tabela de scancode para caractere (simplificada)
static const char scancode_to_char[128] = {
    0,  27,'1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`',  0,'\\',
    'z','x','c','v','b','n','m',',','.','/',  0, '*', 0, ' ', 0,
};

// Entrada de um byte da porta (inline asm)
uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Saída de um byte pra porta (pra cursor)
void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Lê texto do teclado e escreve na tela
void inputp(char* buffer, int maxlen) {
    int i = 0;
    while (i < maxlen - 1) {
        uint8_t scancode = inb(0x60);
        if (scancode & 0x80) continue; // ignora key release

        char c = scancode_to_char[scancode];
        if (c == '\n') break;
        if (c == '\b' && i > 0) {
            i--;
            cursor_col--;
            if (cursor_col == 0 && cursor_row > 0) {
                cursor_row--;
                cursor_col = 79;
            }
            printp("\b "); // limpa da tela
            printp("\b");
            continue;
        }
        if (c) {
            buffer[i++] = c;
            char str[2] = {c, 0};
            printp(str);
        }
    }
    buffer[i] = '\0';
    update_cursor();
}

// shell.h
#ifndef SHELL_H
#define SHELL_H



char* current_directory = "root";

char* directories[] = {
    "root",
    "home",
    "documents",
    "downloads",
    "bin",
    NULL
};

// Atualiza o cursor na tela


// Limpa a tela VGA
void clear_screen() {
    for (int i = 0; i < 25 * 80; i++) {
        vga_buffer[i] = (0x0F << 8) | ' ';
    }
    cursor_row = 0;
    cursor_col = 0;
    update_cursor();
}

// Lista os diretórios disponíveis
void ls() {
    printp("\nConteúdo de ");
    printp(current_directory);
    printp(":\n");
    for (int i = 0; directories[i] != NULL; i++) {
        printp("- ");
        printp(directories[i]);
        printp("\n");
    }
}

// Muda para um diretório se ele existir
void cd(char* dir) {
    for (int i = 0; directories[i] != NULL; i++) {
        if (strcmp(directories[i], dir) == 0) {
            current_directory = directories[i];
            printp("\nDiretório alterado para ");
            printp(current_directory);
            printp("\n");
            return;
        }
    }
    printp("\nErro: diretório não encontrado.\n");
}

// Mostra o diretório atual
void pwd() {
    printp("\nDiretório atual: ");
    printp(current_directory);
    printp("\n");
}

// Mostra o nome do usuário
void whoami() {
    printp("\nUsuário: dfs_user\n");
}

// Mostra a data fake
void date() {
    printp("\nHoje é 14 de Julho de 2025\n");
}

// Mostra info sobre o DFS
void dfs_info() {
    printp("\nDFS - dUnix From Scratch\n");
    printp("Mini sistema operacional educacional\n");
    printp("Feito por Cal, movido a café e autoconhecimento\n");
}

// Mostra comandos disponíveis
void help() {
    printp("\nComandos disponíveis:\n");
    printp("ls         - lista diretórios\n");
    printp("cd <dir>   - muda de diretório\n");
    printp("clear      - limpa a tela\n");
    printp("pwd        - mostra o diretório atual\n");
    printp("whoami     - mostra o nome do usuário\n");
    printp("echo <msg> - imprime uma mensagem\n");
    printp("date       - data atual (fake)\n");
    printp("dfs        - sobre o sistema\n");
    printp("help       - lista os comandos\n");
}

// Analisa e executa o comando digitado
void execute_command(const char* input) {
    if (strcmp(input, "ls") == 0) {
        ls();
    } else if (strncmp(input, "cd ", 3) == 0) {
        cd((char*)(input + 3));
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
    } else if (strcmp(input, "pwd") == 0) {
        pwd();
    } else if (strcmp(input, "whoami") == 0) {
        whoami();
    } else if (strncmp(input, "echo ", 5) == 0) {
        printp("\n");
        printp(input + 5);
        printp("\n");
    } else if (strcmp(input, "date") == 0) {
        date();
    } else if (strcmp(input, "dfs") == 0) {
        dfs_info();
    } else if (strcmp(input, "help") == 0) {
        help();
    } else {
        printp("\nComando não reconhecido. Digite 'help'.\n");
    }
}

// Função principal do shell
void k_main() {
    printp("DFS - dUnix From Scratch v0.1\n");
    printp("Digite 'help' para ver os comandos.\n");

    while (1) {
        printp("\n[");
        printp(current_directory);
        printp("]$ ");

        char input_buffer[128];
        inputp(input_buffer, 128);
        execute_command(input_buffer);
    }
}

#endif
