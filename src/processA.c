/*
	Naveed Manzoor Afridi	| 5149575 |
	Abdul Rauf		| 5226212 |

*/

#include "./../include/processA_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <bmpfile.h>
#include <math.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdarg.h>

// Define the size of the shared memory
#define WIDTH 1600
#define HEIGHT 600
#define DEPTH 4

// Define Log file
#define LOG_PROCESS_A_FILE "log/processA.log"

// Define the struct of the shared memory
struct shared
{
    int x;
    int y;
    int m[WIDTH][HEIGHT];
};

// Pointer to the shared memory
struct shared *shm_ptr;

// declare shered memory size and name
size_t SHARED_MEMORY_SIZE = sizeof(struct shared);
const char* SHARED_MEMORY_NAME = "/my_shm";

// Set the color of the circle (0 - 255)
const u_int8_t RED = 0;
const u_int8_t GREEN = 0;
const u_int8_t BLUE = 255;
const u_int8_t ALPHA = 0;

// Delcare circle radius
const int RADIUS = 30;

// Define the semaphores
sem_t *semaphore;
sem_t *semaphore2;

// File descriptor for the log file
int log_fd;
// File descriptor for the shared memory
int shm_fd;

void create_shared_memory()
{
    // Open the shared memory
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("Error in shm_open");
        exit(1);
    }

    // Set the size of the shared memory
    if (ftruncate(shm_fd, SHARED_MEMORY_SIZE) == -1)
    {
        perror("Error in ftruncate");
        exit(1);
    }

    // Map the shared memory to the memory space of the process
    shm_ptr = mmap(NULL, SHARED_MEMORY_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED)
    {
        perror("Error in mmap");
        exit(1);
    }
}

void create_semaphore(sem_t **sem, const char *name, unsigned int value)
{
    *sem = sem_open(name, O_CREAT, 0666, value);
    if (*sem == SEM_FAILED)
    {
        perror("sem_open failure");
        exit(1);
    }
}

// Function to draw a circle
void draw_my_circle(int radius, int x, int y, bmpfile_t *bmp, rgb_pixel_t color) {
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++) {
        for (int j = centerY - radius; j <= centerY + radius; j++) {
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2)) {
                // Color the pixel at the specified (x,y) position with the given pixel values
                bmp_set_pixel(bmp, i, j, color);
            }
        }
    }
}

// Function to clear the circle
void clear_circle(int radius, int x, int y, bmpfile_t *bmp) {
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;
    // Define the color of the circle
    rgb_pixel_t color = {255, 255, 255, 0}; // White

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++) {
        for (int j = centerY - radius; j <= centerY + radius; j++) {
            // If the pixel is inside the circle..
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2)) {
                // Color the pixel at the specified (x,y) position with the given pixel values
                bmp_set_pixel(bmp, i, j, color);
            }
        }
    }
}



// Function to write to the log file
void log_message(const char *format, ...)
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[100];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", timeinfo);

    va_list args;
    va_start(args, format);

    dprintf(log_fd, "%s", buffer);
    vdprintf(log_fd, format, args);
    va_end(args);
}

void open_log()
{
    // Open the log file
    log_fd = open(LOG_PROCESS_A_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (log_fd == -1)
    {
        perror("Error opening log file");
        exit(1);
    }
    log_message("---- Process A Started---- \n");
}
void process_loop()
{
    // Create the bitmap
    bmpfile_t *bmp;
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    if (bmp == NULL)
    {
        printf("Error: unable to create bitmap\n");
        exit(1);
    }
    
    while (TRUE)
    {

        // Get the mouse event
        int cmd = getch();

        // Get the position of the circle
        int x = circle.x;
        int y = circle.y;

        // If the user resize the window..
        if (cmd == KEY_RESIZE)
        {
            reset_console_ui();
        }
        // Else, if user presses print button..
        else if (cmd == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    mvprintw(LINES - 1, 1, "Print button pressed");

                    // Write to the log file
                    log_message("PROCESS_A: Print button pressed \n");

                    // Save the bitmap
                    bmp_save(bmp, "out/image.bmp");

                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        // If input is an arrow key, move circle accordingly...
        else if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN) {
            // Wait for the semaphore
            sem_wait(semaphore);

            // Write to the log file
            log_message("PROCESS_A: Keyboard button pressed: x: %d - y: %d \n", x, y);

            // Move the circle
            if (cmd == KEY_LEFT)
                x--;
            else if (cmd == KEY_RIGHT)
                x++;
            else if (cmd == KEY_UP)
                y--;
            else if (cmd == KEY_DOWN)
                y++;

            
            // Move the circle
            move_circle(cmd);

            // Draw the circle
            draw_circle();
            
            // Draw the new circle position and update the shared memory
            clear_circle(RADIUS, shm_ptr->x, shm_ptr->y, bmp);
            draw_my_circle(RADIUS, x, y, bmp, (rgb_pixel_t){RED, GREEN, BLUE, ALPHA});
            shm_ptr->x = x;
            shm_ptr->y = y;

            // Signal the semaphore2
            sem_post(semaphore2);

        }
    }
    log_message("---- Process A terminated---- ");
}

int main(int argc, char *argv[])
{
    // Open the log file
    open_log();

    // open and create shared memory
    create_shared_memory();

    // Open the semaphores
    create_semaphore(&semaphore, "/my_sem1", 1);
    create_semaphore(&semaphore2, "/my_sem2", 1);
    
    // Initialize UI
    init_console_ui();
    
    // Infinite loop
    process_loop();
    
    // Close the semaphores and unlink the shared memory
    sem_close(semaphore);
    sem_close(semaphore2);
    sem_unlink("/my_sem1");
    sem_unlink("/my_sem2");
    munmap(shm_ptr, sizeof(struct shared));
    shm_unlink("/my_shm");
    close(shm_fd);
    close(log_fd);

    return 0;
}
