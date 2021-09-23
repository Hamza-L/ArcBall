//
// Created by Hamza Lahmimsi on 2021-02-19.
//

#ifndef VULKANTESTING_NEWVULKANAPP_H

#define VULKANTESTING_NEWVULKANAPP_H


#include "vulkanWindow.h"
#include "vulkanPipeline.h"
#include "vulkanDevice.h"
#include "vulkanSwapChain.h"
#include "vulkanModel.h"
#include "shapes/Plane.h"
#include "shapes/Cube.h"
#include "shapes/Icosahedron.h"
#include "shapes/ObjImporter.h"
#include "shapes/CheckerPlane.h"
#include "stb_image.h"
#include "Scene.h"

//std
#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <filesystem>

//imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"


namespace hva {

    class NewVulkanApp {
    public:
        static constexpr int WIDTH = 1.5*800;
        static constexpr int HEIGHT = 1.5*600;

        NewVulkanApp();
        ~NewVulkanApp();

        NewVulkanApp(const NewVulkanApp&) = delete;
        NewVulkanApp &operator=(const NewVulkanApp&) = delete;

        void run(std::string object);
        void rename(std::string name){
            vulkanWindow.rename(name);
        };
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void mouse_callback(GLFWwindow* window, int button, int action, int mods);
        static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        Node subdivideNode ( Node sourceNode);
    private:
        ImDrawData* draw_data;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        struct UboVP{
            glm::mat4 V;
            glm::mat4 P;
            glm::vec4 lightPos;
            glm::vec4 colour = glm::vec4(0.005f,0.08f,0.12f,1.0f);
            glm::float32_t spec = 32.0f;
        }uboVP;

        struct light{
            glm::vec4 position;
            glm::vec4 colour;
            glm::float32_t intensity;
        };

        glm::mat4 M1 = glm::mat4(1.0f);
        glm::mat4 M2 = glm::mat4(1.0f);
        void loadModels();
        void loadScene();
        void createPipelineLayout();
        void createDescriptorSetLayout();
        void createPushConstantRange();
        void createPipeline(int flag);
        void createCommandBuffers();
        void recordCommand(uint32_t imageIndex);
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void updateUniformBuffers(uint32_t imageIndex);
        void allocateDynamicBufferTransferSpace();
        void recreateSwapchain();
        void drawFrame();
        void updateModels();
        stbi_uc * loadTextureFile(std::string filename, int* width, int* height, VkDeviceSize* imageSize);
        int createTextureImage(std::string filename, std::string norm); //return id of the texture array
        int createTexture(std::string fileName, std::string norm);
        void createTextureSampler();
        int createTextureDescriptor(VkImageView texImage);
        int createTextureAndNormDescriptor(VkImageView texImage, VkImageView normImage);
        void center(Node* object);
        void init_imgui();
        void imGuiParametersSetup();
        static std::vector<std::string> objectfiles();
        static std::vector<std::string> textureFiles();
        void addModel(Node object, std::string texture, std::string normalTex);
        void addModelAt(Node object, int indx, std::string texture, std::string normalTex);

        static void check_vk_result(VkResult err)
        {
            if (err == 0)
                return;
            fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
            if (err < 0)
                abort();
        }

        int firstTex;
        int currPipeline = 0;

        //arcball code
        glm::vec3 getVectorFromMouse(double mouseX, double mouseY);

        std::vector<Vertex> getNormals(std::vector<Vertex> shape);

        VulkanWindow vulkanWindow{WIDTH, HEIGHT, "VulkanApp"};
        VulkanDevice device{vulkanWindow};
        std::unique_ptr<VulkanSwapChain> vulkanSwapChain;

        PipelineConfigInfo pipelineConfig;

        std::vector<std::unique_ptr<VulkanPipeline>> vulkanPipelines;
        int pipelineIndex = 0;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;

        std::vector<VkBuffer> vpUniformBuffer; //create one for every command buffer (swapchain image) so we are assured we are not modifying the buffer as it is being read.
        std::vector<VkDeviceMemory> vpUniformBufferMemory;
        std::vector<VkBuffer> lightUniformBuffer; //create one for every command buffer (swapchain image) so we are assured we are not modifying the buffer as it is being read.
        std::vector<VkDeviceMemory> lightUniformBufferMemory;

        std::vector<VkBuffer> mDynUniformBuffer;
        std::vector<VkDeviceMemory> mDynUniformBufferMemory;

        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorSetLayout samplerSetLayout;
        VkPushConstantRange pushConstantRange;

        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;
        VkDescriptorPool samplerDescriptorPool;
        std::vector<VkDescriptorSet> samplerDescriptorSets; //these ones are not 1:1 with each image but with each textures.
        VkDescriptorPool ImGuiDescriptorPool;
        std::vector<VkDescriptorSet> ImGuiDescriptorSets;
        VkResult err;

        //size_t modelUniformAlignment;
        //pushObject *modelTransferSpace;

        std::vector<std::unique_ptr<VulkanModel>> modelList;
        int currCamera = 0;
        std::vector<int> objectToRender = {0};
        std::vector<int> pipelineToUse = {};
        glm::mat4 view;

        //Scene scene = Scene();

        //assets
        std::vector<VkImage> textureImages;
        std::vector<VkDeviceMemory> textureImageMemory;
        std::vector<VkImageView> textureImageViews;
        std::vector<VkImage> textureNormImages;
        std::vector<VkDeviceMemory> textureNormImageMemory;
        std::vector<VkImageView> textureNormImageViews;
        VkSampler texSampler;

        //arcBall variables
        glm::vec3 mouseInit, mouseCurr;
        std::vector<std::string> items;
        std::vector<std::string> texturesPath;

        //scene object
        Scene scene;
        std::vector<light> lights;
    };
}

#endif //VULKANTESTING_NEWVULKANAPP_H
