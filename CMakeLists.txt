cmake_minimum_required(VERSION 3.16) 
project(game)

# Use system-provided SDL (e.g. from Nix) instead of vendored/ sources
option(USE_SYSTEM_SDL "Use system SDL3 / SDL3_image instead of vendored/" OFF)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# copy assets & data
file(COPY assets DESTINATION .)
file(COPY data DESTINATION .)

if(USE_SYSTEM_SDL)
  find_package(SDL3 REQUIRED CONFIG)
  find_package(SDL3_image REQUIRED CONFIG)
else()
  # This assumes the SDL source is available in vendored/SDL
  add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

  # This assumes the SDL_image source is available in vendored/SDL_image
  add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
endif()

# on Web targets, we need CMake to generate a HTML webpage. 
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

# Create your game executable target as usual
add_executable(main src/main.cpp)

# Link to the actual SDL3 library.
target_link_libraries(main PRIVATE SDL3_image::SDL3_image SDL3::SDL3)

if(EMSCRIPTEN)
  # put files into web assembly
  target_link_libraries(main PRIVATE
      "--preload-file assets"
      "--preload-file data"
  )
endif()
