
#	Naveed Manzoor Afridi	| 5149575 |
#	Abdul Rauf		| 5226212 |
#	Group Name		| NR007   |
# ARP Assignment No. 02

## Description of the programs
The project consists in the implementation of the simulated vision system through shared memory, according to the requirements specified in the PDF file of the assignment.

This second Advanced and Robot Programming (ARP) assignment consists in creating two processes, one that simulates the acquirement of an image and the other one that processes it, trying to find the position of the centre of a circle inside of the image, such as in the out/image.bmp picture.

The two processes involved in the simulation of the vision system, implemented as simple *ncurses windows*, are called **processA** and **processB** and they communicate each others with an inter-process communication pipeline, that is the shared memory (SHM).  
The user can control the *marker* in the **processA** window using the keyboard arrow, and in the **processB** window are printed the trajectory of the movement.  
There is also a **print button**: when the user click on it with the mouse, the program prints a *.png* image of the current position of the *marker*, and put this image into the *out* folder.

There is also a **master** process already prepared for you, responsible of spawning the entire simulation.

## Folders content

The repository is organized as it follows:
- the `src` folder contains the source code for all the processes
- the `include` folder contains all the data structures and methods used within the ncurses framework to build the two GUIs

After compiling the program other two directories will be created:

- the `bin` folder contains all the executable files
- the `out` folder will contain the saved image as a *bmp* file

## Processes
The program is composed of 3 processes:
-  `processA.c` will create a *ncurses* window in which a representation of the circle is displayed. It will also create a *bitmap* file containing the circle in the given position and, whenever the circle changes position, it will communicate it to `processB.c` by updating a copy of the bitmap in the shared memory. Furthermore, when clicking the button **P**, a copy of the image will be saved in the `out` folder.
-  `processB.c` will create a *ncurses* window in which the centre of the circle is displayed. To do so, it will create a local copy of the bitmap, update it by reading the shared memory and compute the centre of the circle.
-  `master.c` will create the mutex semaphore that the two processes need to read and write consistent data and then launch the two processes as children. It will also monitor the status of the two processes and, in case one of them terminates unexpectedly, it will kill the other one and print an error message.


## How to compile and run it
I added one file .sh in order to simplify compiling and running all the processes.  
To compile and run it: execute ```compile_run.sh```;  
To correctly run the programs, you also need to install the *libbitmap* library (see the following paragraph).

## *libbitmap* installation and usage
To work with the bitmap library, you need to follow these steps:
1. Download the source code from [this GitHub repo](https://github.com/draekko/libbitmap.git) in your file system.
2. Navigate to the root directory of the downloaded repo and run the configuration through command ```./configure```. Configuration might take a while.  While running, it prints some messages telling which features it is checking for.
3. Type ```make``` to compile the package.
4. Run ```sudo make install``` to install the programs and any data files and documentation (sudo permission is required).
5. Upon completing the installation, check that the files have been properly installed by navigating to ```/usr/local/lib```, where you should find the ```libbmp.so``` shared library ready for use.
6. In order to properly compile programs which use the *libbitmap* library, you first need to notify the **linker** about the location of the shared library. To do that, you can simply add the following line at the end of your ```.bashrc``` file:      
```export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"```
