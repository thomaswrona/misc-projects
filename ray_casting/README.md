https://lodev.org/cgtutor/raycasting.html

https://www.youtube.com/watch?v=2IEUa2gYJHQ

daniel hirsch

g++ raycaster_untextured.cpp -o mp $(sdl2-config --cflags --libs) && ./mp

g++ raycaster_textured.cpp -o mp $(sdl2-config --cflags --libs) && ./mp

g++ raycaster_textured_floors.cpp -o mp $(sdl2-config --cflags --libs) $(pkg-config --cflags --libs SDL2_image) && ./mp