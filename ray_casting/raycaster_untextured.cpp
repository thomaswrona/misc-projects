#include<SDL2/SDL.h>
#include<SDL2/SDL_events.h>
#include <vector>
#include<stdbool.h>
#include <cmath>
#include<float.h>


#define WIDTH 1000
#define HEIGHT 600
#define COLOR 0xffffffff
#define CELL_SIZE 50
#define FOV 66.0
#define FOV_R FOV / 180.0 * M_PI
#define BASE_HEIGHT 500
#define TURN_SPEED 0.02
#define MOVE_SPEED 0.02

bool app_running{true};
std::vector<std::vector<int>> map = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Collision detection function including outside of map
bool is_collision(int x, int y){
    return x < 0 || x >= (int)map[0].size() || y < 0 || y >= (int)map.size() || map[y][x] != 0;
}

// Wall detection function, must be inside map
bool is_wall(int x, int y){
    return x >= 0 && x < (int)map[0].size() && y >= 0 && y < (int)map.size() && map[y][x] != 0;
}

// See if a point is within buffer distance of any wall
bool collides(double x, double y, double buffer){
    return is_collision(floor(x - buffer), floor(y - buffer)) 
        || is_collision(floor(x + buffer), floor(y - buffer))
        || is_collision(floor(x - buffer), floor(y + buffer))
        || is_collision(floor(x + buffer), floor(y + buffer));
}

int Colors[4]{
    0xFF0000, // Red
    0x00FF00, // Green
    0x0000FF, // Blue
    0xFFFFFF  // White
};

int main(int argc, char *argv[]){
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    SDL_Surface *surface = SDL_GetWindowSurface(window);

    double loc_x{2.5}, loc_y{2.5};
    double look_th{0};

    while(app_running){
        SDL_Event event;
        // Check for quit event
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_QUIT:
                    app_running = false;
                    break;
            }
        }

        //DDA
        //for every vertical slice of frame
        for(int curr_x{0}; curr_x < WIDTH; curr_x++){
            // Draw a default color to reset column to black
            SDL_Rect rectangle{curr_x, 0, 1, HEIGHT};
            SDL_FillRect(surface, &rectangle, 0x000000);

            // Calculate ray direction
            double delta_ray = (2.0 * curr_x / (WIDTH - 1) - 1) * FOV_R / 2;
            double ray_th = fmod(look_th + delta_ray + 2*M_PI, 2*M_PI);

            // Location of current point in map coordinates
            int map_x = floor(loc_x);
            int map_y = floor(loc_y);

            // Decompose ray into components
            double ray_x = cos(ray_th);
            double ray_y = sin(ray_th);

            // Which direction map coordinates change
            int step_x = (ray_th > M_PI/2 && ray_th < 3*M_PI/2) ? -1 : 1;
            int step_y = (ray_y < 0) ? -1 : 1;

            // Calculate distances for ray stepping
            double dist_x = (ray_x != 0.0) ? std::abs(1.0 / ray_x) : DBL_MAX - 1;
            double dist_y = (ray_y != 0.0) ? std::abs(1.0 / ray_y) : DBL_MAX - 1;

            // This is a running distance the ray is moving, based on whether the discrete step is in x or y direction
            double side_x = (ray_th > M_PI/2 && ray_th < 3*M_PI/2) ? (loc_x - map_x) * dist_x : (map_x + 1 - loc_x) * dist_x;
            double side_y = (ray_y < 0) ? (loc_y - map_y) * dist_y : (map_y + 1 - loc_y) * dist_y;

            int side = 0;

            // assumes we are not in a wall, which should never be true unless starting there
            // continue until collision
            do{
                // whichever distance is closer will be stepped up
                if(side_x < side_y){
                    map_x += step_x; // move map coordinate
                    side_x += dist_x; // update running distance
                    side = 0; // track which side in case of hit
                } else {
                    map_y += step_y; // move map coordinate
                    side_y += dist_y; // update running distance
                    side = 1; // track which side in case of hit
                }
            } while(!is_collision(map_x, map_y));
            
            // get correct distance
            double distance = (side == 0) ? side_x - dist_x : side_y - dist_y;
            // perpendicular distance to the wall based on angle from center ray
            double perp_dist = distance * cos(delta_ray);

            if(is_wall(map_x, map_y)){
                // height from distance
                int height = ceil(BASE_HEIGHT / abs(perp_dist));
                // draw the wall slice
                SDL_Rect rectangle{curr_x, (int) fmax(HEIGHT/2 - height/2,  0.0), 1, height};
                SDL_FillRect(surface, &rectangle, side == 0 ? Colors[map[map_y][map_x] - 1] : Colors[map[map_y][map_x] - 1] / 15 * 7);
            }
        }

        double wall_buffer = 0.2; //keep away from walls this distance to prevent clipping

        // movement, tank controls
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT])
            look_th = fmod(look_th - TURN_SPEED + 2*M_PI, 2*M_PI);
        if(keys[SDL_SCANCODE_RIGHT])
            look_th = fmod(look_th + TURN_SPEED + 2*M_PI, 2*M_PI);
        // up and down keep a buffer distance from walls
        if(keys[SDL_SCANCODE_UP]){
            if(!collides(loc_x, loc_y + sin(look_th) * MOVE_SPEED, wall_buffer))
                loc_y += sin(look_th) * MOVE_SPEED;

            if(!collides(loc_x + cos(look_th) * MOVE_SPEED, loc_y, wall_buffer))
                loc_x += cos(look_th) * MOVE_SPEED;
        }
        if(keys[SDL_SCANCODE_DOWN]){
            if(!collides(loc_x, loc_y - sin(look_th) * MOVE_SPEED, wall_buffer))
                loc_y -= sin(look_th) * MOVE_SPEED;

            if(!collides(loc_x - cos(look_th) * MOVE_SPEED, loc_y, wall_buffer))
                loc_x -= cos(look_th) * MOVE_SPEED;
        }
            
        SDL_UpdateWindowSurface(window);
        SDL_Delay(10);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}