include(FetchContent)

set(SDL_TEST        OFF CACHE INTERNAL "")
set(SDL_TESTS       OFF CACHE INTERNAL "")
set(SDL_INSTALL     OFF CACHE INTERNAL "")

FetchContent_Declare(
    sdl3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.14
)

FetchContent_MakeAvailable(sdl3)