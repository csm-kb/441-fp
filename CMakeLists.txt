cmake_minimum_required(VERSION 3.17)
project(fp)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

set(LIB_DIR "${PROJECT_SOURCE_DIR}/lib")

#set(WBIT_SIZE "x86_64")
set(WBIT_SIZE "i686")

set(SDL2_PATH "${LIB_DIR}/SDL2-2.0.3/${WBIT_SIZE}-w64-mingw32")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)

add_executable(fp src/main.cpp src/GEngine.cpp include/GEngine.h include/renderer/Renderer.h src/renderer/Renderer.cpp src/renderer/VAO.cpp src/renderer/Shader.cpp include/renderer/Shader.h include/kInputListener.h include/renderer/Camera.h include/util/convert.h src/util/convert.cpp include/kAnimHandler.h)

include_directories("f:/441/common/include" "./include" ${SDL2_INCLUDE_DIR})
target_link_directories(fp PUBLIC "f:/441/common/lib" "${LIB_DIR}" "${LIB_DIR}/SDL2/${WBIT_SIZE}-w64-mingw32/lib")

target_link_libraries(fp ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${SDL2_TTF_LIBRARIES} opengl32 glfw3 glew32.dll gdi32 )

# Linux may require a different line for linking together the included SDL2 library correctly! (this is untested)
# The above compiles successfully on Windows 10 64-bit.