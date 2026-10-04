#include<SDL2/SDL.h>
#include<SDL2/SDL_events.h>
#include<SDL2/SDL_surface.h>
#include<SDL2/SDL_image.h>
#include<iostream>
#include<vector>
#include<stdbool.h>
#include<cmath>
#include<float.h>
#include<string>


#define WIDTH 1000
#define HEIGHT 600
#define TEXTURE_W 64
#define TEXTURE_H 64
#define COLOR 0xffffffff
#define CELL_SIZE 50
#define FOV 66.0
#define FOV_R FOV / 180.0 * M_PI
#define TURN_SPEED 0.02
#define MOVE_SPEED 0.02

bool app_running{true};
std::vector<std::vector<int>> map = {
  {8,8,8,8,8,8,8,8,8,8,8,4,4,6,4,4,6,4,6,4,4,4,6,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {8,0,3,3,0,0,0,0,0,8,8,4,0,0,0,0,0,0,0,0,0,0,0,4},
  {8,0,0,0,0,0,0,0,0,0,8,4,0,0,0,0,0,6,6,6,0,6,4,6},
  {8,8,8,8,0,8,8,8,8,8,8,4,4,4,4,4,4,6,0,0,0,0,0,6},
  {7,7,7,7,0,7,7,7,7,0,8,0,8,0,8,0,8,4,0,4,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,0,0,0,0,0,6},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,0,0,0,0,4},
  {7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,0,6,0,6,0,6},
  {7,7,0,0,0,0,0,0,7,8,0,8,0,8,0,8,8,6,4,6,0,6,6,6},
  {7,7,7,7,0,7,7,7,7,8,8,4,0,6,8,4,8,3,3,3,0,3,3,3},
  {2,2,2,2,0,2,2,2,2,4,6,4,0,0,6,0,6,3,0,0,0,0,0,3},
  {2,2,0,0,0,0,0,2,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {2,0,0,0,0,0,0,0,2,4,0,0,0,0,0,0,4,3,0,0,0,0,0,3},
  {1,0,0,0,0,0,0,0,1,4,4,4,4,4,6,0,6,3,3,0,0,0,3,3},
  {2,0,0,0,0,0,0,0,2,2,2,1,2,2,2,6,6,0,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,2,0,0,0,0,0,2,5,0,5,0,5,0,5,0,5},
  {2,2,0,0,0,0,0,2,2,2,0,0,0,2,2,0,5,0,5,0,0,0,5,5},
  {2,2,2,2,1,2,2,2,2,2,2,1,2,2,2,5,5,5,5,5,5,5,5,5}
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

void load_texture(std::vector<std::vector<int>> &output, const std::string &file_path){
    std::cout << "Loading texture: " << file_path << std::endl;
    SDL_Surface *loaded_surface = IMG_Load(file_path.c_str());
    if(!loaded_surface){
        printf("Failed to load texture %s: %s\n", file_path.c_str(), IMG_GetError());
        output.emplace_back(TEXTURE_W * TEXTURE_H, 0);
        return;
    }
    SDL_Surface *formatted_surface = SDL_ConvertSurfaceFormat(loaded_surface, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(loaded_surface);
    output.emplace_back(TEXTURE_W * TEXTURE_H);
    // copies per row to output vector
    for(int y = 0; y < TEXTURE_H; y++){
        const Uint32 *row = reinterpret_cast<const Uint32 *>(
            static_cast<const Uint8 *>(formatted_surface->pixels) + y * formatted_surface->pitch);
        for(int x = 0; x < TEXTURE_W; x++)
            output.back()[y * TEXTURE_W + x] = static_cast<int>(row[x]);
    }
    SDL_FreeSurface(formatted_surface);
}

Uint32 darken_color(SDL_Surface *surface, Uint32 color){
    Uint8 red, green, blue;
    SDL_GetRGB(color, surface->format, &red, &green, &blue);
    return SDL_MapRGB(surface->format, red * 7 / 15, green * 7 / 15, blue * 7 / 15);
}

int main(int argc, char *argv[]){
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Raycaster", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    int buffer[HEIGHT][WIDTH];
    SDL_Surface *surface = SDL_CreateRGBSurfaceWithFormatFrom(buffer, WIDTH, HEIGHT,
        32, WIDTH * sizeof(int), SDL_PIXELFORMAT_RGB888);

    std::vector<std::vector<int>> texture;
    std::vector<std::string> texture_files = {
        "pics/eagle.png",
        "pics/redbrick.png",
        "pics/purplestone.png",
        "pics/greystone.png",
        "pics/bluestone.png",
        "pics/mossy.png",
        "pics/wood.png",
        "pics/colorstone.png"
    }; 
    int floor_texture = 3;
    int ceiling_texture = 6;
    
    for(int i{0}; i < 8; i++){
        load_texture(texture, texture_files[i]);
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
        //floor casting
        double left_ray = fmod(look_th - tan(FOV_R / 2) + 2*M_PI, 2*M_PI);
        double right_ray = fmod(look_th + tan(FOV_R/2) + 2*M_PI, 2*M_PI);
        double left_x = cos(look_th) - sin(look_th) * (-tan(FOV_R / 2));
        double right_x = cos(look_th) - sin(look_th) * (tan(FOV_R / 2));
        double left_y = sin(look_th) + cos(look_th) * (-tan(FOV_R / 2));
        double right_y = sin(look_th) + cos(look_th) * (tan(FOV_R / 2));

        for(int y{HEIGHT/2}; y < HEIGHT; y++){
            int p = y - HEIGHT/2;
            double look_z = 0.5 * HEIGHT; //where camera will be in z axis
            double floor_distance = look_z / p; // how far the floor is using the direction camera is looking

            // step between floor rays to cover whole width
            double floor_step_x = floor_distance * (right_x - left_x) / WIDTH;
            double floor_step_y = floor_distance * (right_y - left_y) / WIDTH;

            // starting (left) floor rays
            double floor_x = loc_x + floor_distance * left_x;
            double floor_y = loc_y + floor_distance * left_y;

            for(int x{0}; x < WIDTH; x++, floor_x += floor_step_x, floor_y += floor_step_y){
                // map position of starting rays
                int cell_x = floor(floor_x);
                int cell_y = floor(floor_y);

                // convert position within a cell to texture coordinates
                int texture_x = (int)(TEXTURE_W * (floor_x - cell_x)) % TEXTURE_W;
                int texture_y = (int)(TEXTURE_H * (floor_y - cell_y)) % TEXTURE_H;

                //darkened floor color, the int value there is half values for RGB
                Uint32 color = (texture[floor_texture][texture_x + texture_y * TEXTURE_W] >> 1) & 8355711;
                buffer[y][x] = color;

                //darkened ceiling color
                color = (texture[ceiling_texture][texture_x + texture_y * TEXTURE_W] >> 1) & 8355711;
                buffer[HEIGHT - 1 -y][x] = color;
            }
        }

        //DDA
        //for every vertical slice of frame
        for(int curr_x{0}; curr_x < WIDTH; curr_x++){
            // Calculate ray direction
            double delta_ray = (2.0 * curr_x / (WIDTH - 1) - 1) * tan(FOV_R / 2);
            double ray_th = fmod(look_th + delta_ray + 2*M_PI, 2*M_PI);

            // Location of current point in map coordinates
            int map_x = floor(loc_x);
            int map_y = floor(loc_y);

            // Decompose ray into components
            double ray_x = cos(look_th) - sin(look_th) * delta_ray;
            double ray_y = sin(look_th) + cos(look_th) * delta_ray;

            // Which direction map coordinates change
            int step_x = (ray_x < 0) ? -1 : 1;
            int step_y = (ray_y < 0) ? -1 : 1;

            // Calculate distances for ray stepping
            double dist_x = (ray_x != 0.0) ? std::abs(1.0 / ray_x) : DBL_MAX - 1;
            double dist_y = (ray_y != 0.0) ? std::abs(1.0 / ray_y) : DBL_MAX - 1;

            // This is a running distance the ray is moving, based on whether the discrete step is in x or y direction
            double side_x = (ray_x < 0) ? (loc_x - map_x) * dist_x : (map_x + 1 - loc_x) * dist_x;
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
                int which_texture = map[map_y][map_x] - 1;

                //might be perp_dist?
                double wall_x = (side == 0) ? fmod(loc_y + distance * ray_y, 1.0) : fmod(loc_x + distance * ray_x, 1.0);
                int tex_x = (int)floor(wall_x * TEXTURE_W) % TEXTURE_W;
                // flip if direction is negative coordinate
                if(side == 0 && ray_x > 0 || side == 1 && ray_y < 0)
                    tex_x = TEXTURE_W - tex_x - 1;
                // height from distance
                int height = ceil(HEIGHT / abs(distance));
                // draw the wall slice
                double texture_height_rat = (double)TEXTURE_H / (double)height;
                double start_in_texture = (std::max((HEIGHT - height)/2,  0) + (double)(height - HEIGHT)/2) * texture_height_rat;
                int tex_y;
                for(int y{std::max((HEIGHT - height)/2,  0)}; y < std::min((HEIGHT + height)/2, HEIGHT); y++){
                    tex_y = (int) (start_in_texture + (y - std::max((HEIGHT - height)/2,  0)) * texture_height_rat) % TEXTURE_H;
                    Uint32 color = texture[which_texture][tex_y * TEXTURE_W + tex_x];
                    if(side == 1){
                        color = darken_color(surface, color);
                    }
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