cmake_minimum_required(VERSION 3.28)
project(MyVulkan)
set(CMAKE_CXX_STANDARD 20)
include_directories(include)
link_directories(${PROJECT_SOURCE_DIR}/LIB)


add_executable(MyVulkan src/main.cpp
        src/stb_image.cpp
        Include/VKBase.h
        Include/EasyVulkanStart.h
        Include/GlfwGeneral.hpp
        Include/tools.hpp
        Include/EasyVulkan.hpp)
target_link_libraries(MyVulkan PUBLIC glfw3)
target_link_libraries(MyVulkan PUBLIC vulkan-1)
