#include<SDL2/SDL.h>
#include<SDL2/SDL_events.h>
#include <vector>
#include<stdbool.h>

#include<iostream>


#define WIDTH 1000
#define HEIGHT 600
#define COLOR 0xffffffff
#define CELL_SIZE 50
#define FOV 66.0

bool app_running{true};
std::vector<std::vector<int>> map = {
    {0,0,0,1,0},
    {0,0,1,1,0},
    {1,0,0,1,1},
    {1,0,0,0,1},
    {1,1,1,0,1}
};

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);


    double loc_x{1}, loc_y{1};
    double look_x{1}, look_y{0};

    int counter{0};
    while(app_running){
        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    app_running = false;
                    break;
            }
        }

        SDL_Rect rectangle{WIDTH/2, HEIGHT/2, 100+counter, 100+counter++};
        SDL_FillRect(surface, &rectangle, COLOR);
        SDL_UpdateWindowSurface(window);

        SDL_Delay(10);
    }


    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}