#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAX_TODO 100
#define MAX_TODO_LENGTH 100

typedef struct {
    int id;
    char name[MAX_TODO_LENGTH];
} Todo;

Todo todos[MAX_TODO];
int len_todo = 0;

void printf(const char *format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);

    while(*format != '\0') {
        if (*format != '%') {
            asm (
                "mov x0, 1\n"
                "mov x1, %0\n"
                "mov x2, 1\n"
                "mov x8, 64\n"
                "svc 0\n"
                :
                : "r" (format++)
                : "x0", "x1", "x2", "x8"
            );
            continue;
        }

        format++;

        switch(*format) {
        case 's':
            char *str = __builtin_va_arg(args, char*);
            int len = 0;
            
            while (str[len] != '\0')
                len++;
            
            asm (
                "mov x0, 1\n"
                "mov x1, %0\n"
                "mov x2, %1\n"
                "mov x8, 64\n"
                "svc 0\n"
                :
                : "r" (str), "r" (len)
                : "x0", "x1", "x2", "x8"
            );
            break;
        case 'd': {
            int num = __builtin_va_arg(args, int);
            char num_str[1024]; 
            int len = 0;
            
            do {
                num_str[len++] = '0' + (num % 10);
                num /= 10;
            } while (num != 0);
            
            for (int i = 0; i < len / 2; i++) {
                char temp = num_str[i];
                num_str[i] = num_str[len - i - 1];
                num_str[len - i - 1] = temp;
            }
                    
            asm (
                "mov x0, %0\n"
                "mov x1, %1\n"
                "mov x2, %2\n"
                "mov x8, 64\n" 
                "svc 0\n"      
                :
                : "r" (1), "r" (num_str), "r" (len)
                : "x0", "x1", "x2", "x8"
            );
            break;
        }
        default:
            asm (
                "mov x0, %0\n"
                "mov x1, %1\n"
                "mov x2, %2\n"
                "mov x8, 64\n" 
                "svc 0\n"      
                :
                : "r" (1), "r" (format - 1), "r" (2)
                : "x0", "x1", "x2", "x8"
            );
            break;
        }

        format++;
    }
    __builtin_va_end(args);
}

void scanf(const char *format, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, format);

    char buffer[1024];
    int bytes_read;

    asm (
        "mov x0, 0\n"
        "mov x1, %1\n"
        "mov x2, %2\n"
        "mov x8, 63\n"
        "svc 0\n"
        "mov %0, x0\n"
        : "=r" (bytes_read)
        : "r" (buffer), "r" (sizeof(buffer))
        : "x0", "x1", "x2", "x8"
    );

    buffer[bytes_read] = '\0';

    const char *bptr = buffer;

    while(*format != '\0') {
        if (*format != '%')
            format++;

        format++;

        switch(*format){
        case 's':
            char *str_args = __builtin_va_arg(args, char *);
            while (*bptr != '\n' && *bptr != '\0')
                *str_args++ = *bptr++;
            *str_args = '\0';
            break;

        case 'd':
            int *int_args = __builtin_va_arg(args, int *);
            
            if (int_args == NULL) 
                return;
            
            *int_args = 0;

            while (*bptr >= '0' && *bptr <= '9')
                *int_args = (*int_args * 10) + (*bptr++ - '0');

            break;
        }

        format++;
    }
    __builtin_va_end(args);
}

void __attribute__((noreturn)) exit(int code) {
    asm (
        "mov x0, %0\n"
        "mov x8, 93\n"
        "svc 0"
        :
        : "r" (code)
        : "x0", "x8"
    );

    __builtin_unreachable();
}

void show_task(void) {
    printf("| %s | %s |\n", "ID", "Task");
    printf("--------------------------------------------\n");
    for (int i = 0; i < len_todo; i++)
        printf("| %d | %s |\n", i, todos[i].name);
}


void add_task(void) {
    if (len_todo > MAX_TODO) {
        printf("Maximum todo reached!, Please delete some task to continue.\n");
        return;
    }

    printf("Enter your task name: ");
    scanf("%s", &todos[len_todo].name);
    printf("Task %s successfuly added.\n", todos[len_todo++].name);
}

void del_task(void) {
    int id;
    printf("Which ID task to delete? [eg 0]: ");
    scanf("%d", &id);

    if (id < 0 || id >= len_todo) 
        return printf("ID not found");
    
    if (len_todo == 1) {
        todos[0].name[0] = '\0';
        len_todo = 0;
        return;
    }

    for (int i = id; i < len_todo -1; i++) {
        todos[i] = todos[i + 1];
    }

    len_todo--;
}

void _start(void) {
    int choice;


    printf("\x1b[32m Todo app CLI\x1b[0m\n\x1b[34mWritten by \x1b[33m@miruchigawa\x1b[0m\n");

    for(;;) {
        printf("\x1b[32mUsage:\n\x1b[31m1. Write\n2. List\n3. Delete\n4. Exit\x1b[0m\n󱞩 ");
        scanf("%d", &choice);

        switch(choice) {
        case 1:
            add_task();
            break;
        case 2:
            show_task();
            break;
        case 3:
            del_task();
            break;
        case 4:
            printf("󰈆 Exited.\n");
            exit(0);
        default:
            printf("Invalid choice %d.\n", choice);
            break;
        }
    }
    exit(0);
}
