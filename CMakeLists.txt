cmake_minimum_required(VERSION 3.17)
project(ArcBall)

set(CMAKE_CXX_STANDARD 17)

IF (APPLE)
    SET(GUI_TYPE MACOSX_BUNDLE shapes/vert.h)
ENDIF (APPLE)

#for optimization
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

include_directories(C:/GLFW/include)
include_directories(C:/VulkanSDK/1.2.189.2/Include)
include_directories(C:/GLM)
include_directories(${PROJECT_SOURCE_DIR}/imgui)

#message("message to display")

#add ${GUI_TYPE} after VulkanTesting (ie target) to compile into app
add_executable(ArcBall main.cpp newVulkanApp.cpp vulkanWindow.cpp vulkanWindow.h newVulkanApp.h vulkanPipeline.cpp vulkanPipeline.h vulkanDevice.cpp vulkanDevice.h vulkanSwapChain.cpp vulkanSwapChain.h vulkanModel.cpp vulkanModel.h stb_image.h Scene.cpp Scene.h Camera.h shapes/CheckerPlane.h)

target_sources(ArcBall PUBLIC
        "shapes/ObjImporter.h"
        "shapes/vert.h"
        "shapes/Cube.h"
        "shapes/Triangle.h"
        "shapes/Plane.h"
        "shapes/Node.h"
        "shapes/Icosahedron.h")

target_sources(ArcBall PUBLIC
        "imgui/imgui.h"
        "imgui/imgui.cpp"

        "imgui/imgui_demo.cpp"
        "imgui/imgui_draw.cpp"
        "imgui/imgui_widgets.cpp"
        "imgui/imgui_tables.cpp"

        "imgui/imgui_impl_vulkan.cpp"
        "imgui/imgui_impl_glfw.cpp")

target_link_libraries(ArcBall "C:/GLFW/lib-mingw-w64/libglfw3.a")
target_link_libraries(ArcBall "C:/VulkanSDK/1.2.189.2/Lib/vulkan-1.lib")
target_link_libraries(ArcBall imm32.lib)
target_link_libraries(ArcBall "${PROJECT_SOURCE_DIR}/imgui")
#target_link_libraries(VulkanTesting "/Users/hamzalah/VulkanSDK/macOS/lib/libvulkan.1.dylib")