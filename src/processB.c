/*
	Naveed Manzoor Afridi	| 5149575 |
	Abdul Rauf		| 5226212 |

*/

#include "./../include/processB_utilities.h"
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

// Define Log files
#define LOG_PROCESS_B_FILE "log/processB.log"

// Define struct for shared memory and variables
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

// Define the semaphores
sem_t *semaphore;
sem_t *semaphore2;

// Buffer to store the string to write to the log file
char log_buffer[100];
// File descriptor for the log file
int log_fd;
int shm_fd;

void initialize_shared()
{
    shm_ptr = (struct shared *)calloc(1, sizeof(struct shared));
}

void create_shared_memory()
{
    // Open the shared memory
    shm_fd = shm_open(SHARED_MEMORY_NAME, O_RDWR, 0666);
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
    *sem = sem_open(name, value);
    if (*sem == SEM_FAILED)
    {
        perror("sem_open failure");
        exit(1);
    }
}

void draw_my_circle(int radius, int x, int y, bmpfile_t *bmp, rgb_pixel_t color)
{
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++)
    {
        for (int j = centerY - radius; j <= centerY + radius; j++)
        {
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2))
            {
                // Color the pixel at the specified (x,y) position with the given pixel values
                bmp_set_pixel(bmp, i, j, color);
            }
        }
    }
}

void clear_circle(int radius, int x, int y, bmpfile_t *bmp)
{
    // Define the center of the circle
    int centerX = x * 20;
    int centerY = y * 20;
    // Define the color of the circle
    rgb_pixel_t color = {255, 255, 255, 0}; // White

    // Loop over the pixels of the circle
    for (int i = centerX - radius; i <= centerX + radius; i++)
    {
        for (int j = centerY - radius; j <= centerY + radius; j++)
        {
            // If the pixel is inside the circle..
            if (pow(i - centerX, 2) + pow(j - centerY, 2) <= pow(radius, 2))
            {
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
    log_fd = open(LOG_PROCESS_B_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (log_fd == -1)
    {
        perror("Error opening log file");
        exit(1);
    }
    log_message("---- Process B Started---- \n");
}

void process_loop()
{
    // Utility variable to avoid triggering resize event on launch
    int first_resize = TRUE;

    // Create the bitmap
    bmpfile_t *bmp;
    bmp = bmp_create(WIDTH, HEIGHT, DEPTH);
    if (bmp == NULL)
    {
        printf("Error: unable to create bitmap\n");
        exit(1);
    }

    // Variable declaration in order to store the coordinates of the circles
    int center_cord = 0;
    int x_cord[600];
    int y_cord[600];
    int cont;
    int y_old;
    int x_old;

    // Flag
    int flag;


    // Infinite loop
    while (TRUE)
    {

        // Get the mouse event
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }
        else
        {
            mvaddch(LINES / 2, COLS / 2, '0');
            refresh();

            // Wait for the semaphore 2
            sem_wait(semaphore2);

            // Initialize the coordinates of the circles
            for (int i = 0; i < 600; i++)
            {
                x_cord[i] = 0;
                y_cord[i] = 0;
            }
            center_cord = 0;

            int i, j;
            cont = 0;
            flag = 0;

            // Get the coordinates of the circles
            for (i = 0; i < 1600; i++)
            {

                // If the flag is 1, break the loop
                if (flag == 1)
                {
                    break;
                }
                for (j = 0; j < 600; j++)
                {

                    // Get the coordinates of the circles from the shared memory
                    if (shm_ptr->m[i][j] == 1)
                    {
                        x_cord[cont] = j;
                        y_cord[cont] = i;

                        if (x_cord[cont] > x_cord[cont - 1])
                        {
                            flag = 1;
                            break;
                        }

                        cont++;
                        break;
                    }
                }
            }

            // Update the position of the center
            center_cord = shm_ptr->y;
            y_cord[cont - 1] = shm_ptr->x;

            // Write the position of the center in the log file
            log_message("PROCESS B: Position of center updated: x: %d - y: %d \n", center_cord, (int)y_cord[cont - 1]);

            // Draw the circles
            mvaddch((int)center_cord, (int)y_cord[cont - 1], '0');

            refresh();

            // Signal the semaphore 1
            sem_post(semaphore);

            // Cancel the circle with the coordinates of the previous loop
            clear_circle(30, y_old, x_old, bmp);

            // Choose the circle color
            rgb_pixel_t color = {RED, GREEN, BLUE, ALPHA};

            // Draw the circle with the coordinates of the current loop
            draw_my_circle(30, y_cord[cont - 1], center_cord, bmp, color);

            // Update the (previous) coordinates for the next loop
            y_old = y_cord[cont - 1];
            x_old = center_cord;
        }
    }
    log_message("---- Process B terminated---- ");    
}

int main(int argc, char const *argv[])
{

    //open log file for ProcessB
    open_log();
    
    // create and open shared memory
    create_shared_memory();
    
    // Open the semaphores
    create_semaphore(&semaphore, "/my_sem1", 0);
    create_semaphore(&semaphore2, "/my_sem2", 0);

    // Initialize UI
    init_console_ui();

    // Infinite loop
    process_loop();
    
    // Close the semaphores and unlink the shared memory
    sem_close(semaphore);
    sem_close(semaphore2);
    munmap(shm_ptr, sizeof(struct shared));
    shm_unlink("/my_shm");
    close(shm_fd);
    close(log_fd);
    endwin();
    return 0;
}


