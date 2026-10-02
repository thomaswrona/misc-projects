#include<SDL2/SDL.h>
#include<SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>
#include <vector>
#include<stdbool.h>
#include <cmath>
#include<float.h>


#define WIDTH 1000
#define HEIGHT 600
#define TEXTURE_W 64
#define TEXTURE_H 64
#define COLOR 0xffffffff
#define CELL_SIZE 50
#define FOV 66.0
#define FOV_R FOV / 180.0 * M_PI
#define BASE_HEIGHT 500
#define TURN_SPEED 0.02
#define MOVE_SPEED 0.02

bool app_running{true};
std::vector<std::vector<int>> map = {
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
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

int main(int argc, char *argv[]){
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    int buffer[HEIGHT][WIDTH];
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(buffer, WIDTH, HEIGHT,
        32, WIDTH * sizeof(int), SDL_PIXELFORMAT_RGB888);

    std::vector<std::vector<int>> texture(8, std::vector<int>(TEXTURE_W * TEXTURE_H));
    for(int x{0}; x < TEXTURE_W; x++){
        for(int y{0}; y < TEXTURE_H; y++){
            int xor_color = (x * 256 / TEXTURE_W) ^ (y * 256 / TEXTURE_H);
            int x_color = (x * 256 / TEXTURE_W);
            int y_color = (y * 256 / TEXTURE_H);
            int xy_color = (x_color + y_color) / 2;
            texture[0][y * TEXTURE_W + x] = 65536 * 254 * (x != y && x != TEXTURE_W - y);
            texture[1][y * TEXTURE_W + x] = xy_color * (65536);
            texture[2][y * TEXTURE_W + x] = xor_color * (1 + 256 + 65536);
            texture[3][y * TEXTURE_W + x] = 65536 * 192 * (x % 16 && y % 16);
            texture[4][y * TEXTURE_W + x] = 256 * y_color;
            texture[5][y * TEXTURE_W + x] = y_color;
            texture[6][y * TEXTURE_W + x] = 0xFFFFFF;
            texture[7][y * TEXTURE_W + x] = 0x2E2848;
        }
    }

    double loc_x{4.5}, loc_y{4.5};
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
            
            

            if(is_wall(map_x, map_y)){
                // get correct distance
                double distance = (side == 0) ? side_x - dist_x : side_y - dist_y;
                // perpendicular distance to the wall based on angle from center ray
                double perp_dist = distance * cos(delta_ray);

                int which_texture = map[map_y][map_x] - 1;

                //might be perp_dist?
                double wall_x = (side == 0) ? fmod(loc_y + distance * ray_y, 1.0) : fmod(loc_x + distance * ray_x, 1.0);
                int tex_x = (int)floor(wall_x * TEXTURE_W) % TEXTURE_W;
                // flip if direction is negative coordinate
                if(side == 0 && ray_x > 0 || side == 1 && ray_y < 0)
                    tex_x = TEXTURE_W - tex_x - 1;
                // height from distance
                int height = ceil(BASE_HEIGHT / abs(perp_dist));
                // draw the wall slice
                double texture_height_rat = (double)TEXTURE_H / (double)height;
                double start_in_texture = (std::max((HEIGHT - height)/2,  0) + (double)(height - HEIGHT)/2) * texture_height_rat;
                int tex_y;
                for(int y{std::max((HEIGHT - height)/2,  0)}; y < std::min((HEIGHT + height)/2, HEIGHT); y++){
                    tex_y = (int) (start_in_texture + (y - std::max((HEIGHT - height)/2,  0)) * texture_height_rat) % TEXTURE_H;
                    Uint32 color = texture[which_texture][tex_y * TEXTURE_W + tex_x];
                    if(side == 1)
                        color = color * 7 / 15;
                    buffer[y][curr_x] = color;
                }
            }
        }

        SDL_BlitSurface(surface, NULL, SDL_GetWindowSurface(window), NULL);
        SDL_UpdateWindowSurface(window);

        for(int y{0}; y < HEIGHT; y++)
            for(int x{0}; x < WIDTH; x++)
                buffer[y][x] = 0;

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
            
        SDL_Delay(10);
    }
    
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}