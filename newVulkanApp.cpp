//
// Created by Hamza Lahmimsi on 2021-02-19.
//

#include "newVulkanApp.h"

#define GLFW_INCLUDE_VULKAN

#include <stdexcept>
#include <array>
#include <iostream>
#include <mm_malloc.h>
#include <cstdlib>
#include <glm/gtx/vector_angle.hpp>



const int MAX_OBJECTS = 10;
const bool TEXTURE = true;
bool UP_PRESS = false;
bool DOWN = false;
bool RIGHT = false;
bool LEFT = false;
bool SHIFT = false;
bool COM = false;
float scroll = 0;

//mouse events
static bool MPRESS_R = false;
static bool MPRESS_L = false;
static bool MFLAG_R = true;
static bool MFLAG_L = true;

//arcball parameter;
float FIT = 1.5f;
static float GAIN = 2.0f;


namespace hva{

    NewVulkanApp::NewVulkanApp() {
        createDescriptorSetLayout();
        createPushConstantRange();
        createPipelineLayout();
        recreateSwapchain();
        //firstTex = createTextureImage("test.jpg");
        //allocateDynamicBufferTransferSpace(); only for Dynamic Buffer Uniform Objects
        createTextureSampler();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        loadScene();
        loadModels();
        init_imgui();
    }

    NewVulkanApp::~NewVulkanApp() {
        //_mm_free(modelTransferSpace); //for dynamic Buffer Object


        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();


        vkDestroyDescriptorPool(device.device(), descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device.device(), descriptorSetLayout, nullptr);

        if(TEXTURE) {
            vkDestroySampler(device.device(), texSampler, nullptr);
            vkDestroyDescriptorPool(device.device(), samplerDescriptorPool, nullptr);
            vkDestroyDescriptorSetLayout(device.device(), samplerSetLayout, nullptr);
        }

        vkDestroyDescriptorPool(device.device(), ImGuiDescriptorPool, nullptr);
        //vkDestroyDescriptorSetLayout(device.device(), samplerSetLayout, nullptr);

        for(size_t i=0; i<vulkanSwapChain->imageCount();i++){
            vkDestroyBuffer(device.device(),vpUniformBuffer[i], nullptr);
            vkFreeMemory(device.device(),vpUniformBufferMemory[i], nullptr);
            //vkDestroyBuffer(device.device(),mDynUniformBuffer[i], nullptr);
            //vkFreeMemory(device.device(),mDynUniformBufferMemory[i], nullptr);
        }

        for (size_t i = 0; i < textureImages.size(); i++){
            vkDestroyImageView(device.device(),textureImageViews[i],nullptr);
            vkDestroyImage(device.device(),textureImages[i],nullptr);
            vkFreeMemory(device.device(), textureImageMemory[i], nullptr);
        }

        for (size_t i = 0; i < textureNormImages.size(); i++){
            vkDestroyImageView(device.device(),textureNormImageViews[i],nullptr);
            vkDestroyImage(device.device(),textureNormImages[i],nullptr);
            vkFreeMemory(device.device(), textureNormImageMemory[i], nullptr);
        }

        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    }

    void NewVulkanApp::run(std::string object) {

        glfwSetTime(0);
        while (!vulkanWindow.shouldClose()){
            glfwPollEvents();

            //other event handling


            //imgui new frame
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            //ImGui::ShowDemoWindow(reinterpret_cast<bool *>(true));
            imGuiParametersSetup();
            ImGui::Render();

            draw_data = ImGui::GetDrawData();

            //imgui commands

            glfwSetKeyCallback(vulkanWindow.getWindow(),key_callback);
            glfwSetMouseButtonCallback(vulkanWindow.getWindow(), mouse_callback);
            glfwSetScrollCallback(vulkanWindow.getWindow(), scroll_callback);

            updateModels();
            drawFrame();
        }
        vkDeviceWaitIdle(device.device()); //wait until all GPU operations have been completed
    }

    void NewVulkanApp::loadModels() {

        //addModel(scene,"","");
        Cube c;
        c.transform(glm::mat4(3.0f));
        addModel(c,"pavingStones.jpg","pavingStonesUV.jpg");

        items = objectfiles();
        texturesPath = textureFiles();

        //std::cout<<texturesPath.size()<<std::endl;

        for(std::string objName: items){
            std::cout<<objName<<std::endl;
            ObjImporter object = ObjImporter(objName,glm::vec4(0.1f,0.1f,0.1f,1.0f));
            center(&object);
            addModel(object,object.texName,object.UVTexName);
            //modelList.push_back(std::move(std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture("pavingStones.jpg","pavingStonesUV.jpg"))));
        }


        CheckerPlane p = CheckerPlane(glm::vec4(0.1f,0.1f,0.1f,1.0f),glm::vec4(0.2f,0.2f,0.2f,1.0f),100);
        p.transform(glm::translate(glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(1.0f,0.0f,0.0f)), glm::vec3(0.0f,0.0f,-3.0f)));

        addModel(p,"","");
        //modelList.push_back(std::move(std::make_unique<VulkanModel>(device, p.getVert(), p.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture("pavingStones.jpg","pavingStonesUV.jpg"))));


        //Scene scene;
        //scene.addChild(object);
        

        //setting up the UboVP matrices
        if(scene.getCameras().size()>0){
            uboVP.P = scene.getCameras()[0].camMatProj;
            uboVP.V = scene.getCameras()[0].camMatView;
            uboVP.lightPos = glm::vec4(scene.getLights()[0].lightPos, 1.0f);
        } else {
            uboVP.P = glm::perspective(glm::radians(45.0f), (float) vulkanSwapChain->width()/(float)vulkanSwapChain->height(),0.1f, 100.0f);
            uboVP.V = glm::lookAt(glm::vec3(0.0f,1.5f,4.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
            uboVP.lightPos = glm::vec4(-2.0f, 3.0f, 4.0f, 1.0f);
        }

        //uboVP.lightPos = glm::vec4(-2.0f, 3.0f, 4.0f, 1.0f);
        //std::cout<<uboVP.M[0][0]<<","<<uboVP.M[1][1]<<","<<uboVP.M[2][2]<<","<<uboVP.M[3][3]<<std::endl;

        Icosahedron sphere(14,glm::vec3(1.0f,0.5f,0.0f));


        //glm::mat4 M = glm::translate(glm::rotate(glm::mat4(1.0f),glm::radians(45.0f),glm::vec3(1.0f,1.0f,0.0f)),glm::vec3(1.0f,0.0f,-0.0f));
        //M = glm::scale(M,glm::vec3(0.3f));
        //std::cout<<"the number of vertices is: "<<sphere.getVert().size()<<std::endl;
        //std::cout<<"the number of indices is: "<<sphere.getInd().size()<<std::endl;

        //VulkanModel thing = VulkanModel(device, sphere.getVert(), sphere.getInd() , device.graphicsQueue(), device.getCommandPool(), createTexture("test.jpg","test.jpg"));

        /*
        for(Node child: scene.getChildren()){
            modelList.push_back(std::move(std::make_unique<VulkanModel>(device, child.getVert(), child.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture("pavingStones.jpg","pavingStonesUV.jpg"))));
        }
         */
        //Node scene;
        //Plane p = Plane(glm::rotate(glm::mat4(100.0f),glm::radians(-90.0f),glm::vec3(1.0f,0.0f,0.0f)));
        //CheckerPlane p = CheckerPlane();
        //p.transform(glm::rotate(glm::mat4(1.0f),glm::radians(-90.0f),glm::vec3(1.0f,0.0f,0.0f)));
        //scene.addChild(p);

        //modelList.push_back(std::move(std::make_unique<VulkanModel>(device, sphere.getVert(), sphere.getInd() , device.graphicsQueue(), device.getCommandPool(), createTexture("test.jpg","test.jpg"))));
        //scene.addObject(VulkanModel(device, sphere.getVert(), sphere.getInd() , device.graphicsQueue(), device.getCommandPool(), createTexture("test.jpg","test.jpg")));


        //cube1.transform(glm::mat4(1.2f));
        //modelList.push_back(std::move(std::make_unique<VulkanModel>(device, cube1.getVert(), cube1.getInd() , device.graphicsQueue(), device.getCommandPool())));
        //modelList[1]->setModel(glm::mat4(1.2f));
        //modelList.push c _back(std::move(std::make_unique<VulkanModel>(device, scene.getVert(), scene.getInd()  , device.graphicsQueue(), device.getCommandPool())));

    }

    void NewVulkanApp::addModel(Node object, std::string texture, std::string normalTex) {
        if(texture.empty() && normalTex.empty()){
            modelList.push_back(std::move(std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture("pavingStones.jpg","pavingStonesUV.jpg"))));
            pipelineToUse.push_back(0);
        } else if (!texture.empty() && normalTex.empty()){
            modelList.push_back(std::move(std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture(texture,"pavingStonesUV.jpg"))));
            pipelineToUse.push_back(1);
        } else if (!texture.empty() && !normalTex.empty()){
            modelList.push_back(std::move(std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture(texture,normalTex))));
            pipelineToUse.push_back(2);
        }
    }

    void NewVulkanApp::addModelAt(Node object, int indx,std::string texture, std::string normalTex) {
        if(texture.empty() && normalTex.empty()){
            modelList[indx] = std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture("pavingStones.jpg","pavingStonesUV.jpg"));
            pipelineToUse[indx] = 0;
        } else if (!texture.empty() && normalTex.empty()){
            modelList[indx] = std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture(texture,"pavingStonesUV.jpg"));
            pipelineToUse[indx] = 1;
        } else if (!texture.empty() && !normalTex.empty()){
            modelList[indx] = std::make_unique<VulkanModel>(device, object.getVert(), object.getInd() , device.graphicsQueue(), device.getCommandPool(),createTexture(texture,normalTex));
            pipelineToUse[indx] = 2;;
        }
    }

    void NewVulkanApp::loadScene() {
        scene = Scene("BoxStacks", WIDTH, HEIGHT);
        for(int i = 0; i< scene.getLights().size(); i++){
            light l;
            l.position = glm::vec4(scene.getLights()[0].lightPos,0.0f);
            l.colour = glm::vec4(scene.getLights()[0].colour,1.0f);
            l.intensity = scene.getLights()[0].intensity;
            lights.push_back(l);
        }
    }

    void NewVulkanApp::center(Node* object){
        glm::vec3 max = glm::vec3(-INFINITY);
        glm::vec3 min = glm::vec3(INFINITY);

        for (Vertex v: object->getVert()){
            if(v.position.x>max.x){
                max.x = v.position.x;
            }

            if(v.position.x<min.x){
                min.x = v.position.x;
            }

            if(v.position.y>max.y){
                max.y = v.position.y;
            }

            if(v.position.y<min.y){
                min.y = v.position.y;
            }

            if(v.position.z>max.z){
                max.z = v.position.z;
            }

            if(v.position.z<min.z){
                min.z = v.position.z;
            }
        }

        //std::cout<<min.x<<std::endl;
        //std::cout<<max.x<<std::endl;

        object->transform(glm::translate(glm::mat4(1.0f),glm::vec3(-(max+min)/2.0f)));
    }

    Node NewVulkanApp::subdivideNode(Node sourceNode) {
        Node node1, node2, node3, node4, outNode, tempNode;
        node1 = sourceNode;
        node2 = sourceNode;
        node3 = sourceNode;
        node4 = sourceNode;

        glm::mat4 M = glm::scale(glm::mat4(1.0f),glm::vec3(0.5f,0.5f,0.5f)) * glm::translate(glm::mat4(1.0f),glm::vec3(1.0f,1.0f,0.0f));
        node1.transform(M);
        M = glm::scale(glm::mat4(1.0f),glm::vec3(0.5f,0.5f,0.5f)) * glm::translate(glm::mat4(1.0f),glm::vec3(-1.0f,1.0f,0.0f));
        node2.transform(M);
        M = glm::scale(glm::mat4(1.0f),glm::vec3(0.5f,0.5f,0.5f)) * glm::translate(glm::mat4(1.0f),glm::vec3(1.0f,-1.0f,0.0f));
        node3.transform(M);
        M = glm::scale(glm::mat4(1.0f),glm::vec3(0.5f,0.5f,0.5f)) * glm::translate(glm::mat4(1.0f),glm::vec3(-1.0f,-1.0f,0.0f));
        node4.transform(M);

        tempNode.addChild(node1);
        tempNode.addChild(node2);
        tempNode.addChild(node3);
        tempNode.addChild(node4);

        outNode.addChild(tempNode);

        M =  glm::translate(glm::mat4(1.0f), glm::vec3(0.0f,0.0f,-1.0f));
        tempNode.transform(M);
        outNode.addChild(tempNode);

        return outNode;
    }

    void NewVulkanApp::createPipelineLayout() {

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (TEXTURE) {
            std::array<VkDescriptorSetLayout, 2> descriptorSetLayouts = {descriptorSetLayout, samplerSetLayout};
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        } else {
            std::array<VkDescriptorSetLayout, 1> descriptorSetLayouts = {descriptorSetLayout};
            pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        }


        if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS){
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    void NewVulkanApp::createDescriptorSetLayout() {

        //uniform value descriptor set layout
        VkDescriptorSetLayoutBinding uboVPLayoutBinding = {};
        uboVPLayoutBinding.binding = 0; //binding location in shader
        uboVPLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //type of descriptor (could be: uniform, dynamic uniform, texture ...)
        uboVPLayoutBinding.descriptorCount = 1; //number of objects/descriptor we are binding.
        uboVPLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //shader stage to bind to.
        uboVPLayoutBinding.pImmutableSamplers = nullptr; //sample data can't be changed. we are not using any texture so we are leaving it null. only the sampler becomes immutable, not the image.

        /*
        VkDescriptorSetLayoutBinding uboLightsLayoutBinding = {};
        uboVPLayoutBinding.binding = 1; //binding location in shader
        uboVPLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //type of descriptor (could be: uniform, dynamic uniform, texture ...)
        uboVPLayoutBinding.descriptorCount = 1; //number of objects/descriptor we are binding.
        uboVPLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //shader stage to bind to.
        uboVPLayoutBinding.pImmutableSamplers = nullptr; //sample data can't be changed. we are not using any texture so we are leaving it null. only the sampler becomes immutable, not the image.

        */

        /*
        VkDescriptorSetLayoutBinding modelLayoutBinding = {}; //for the dynamic model uniform buffer.
        modelLayoutBinding.binding = 1;
        modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        modelLayoutBinding.descriptorCount = 1;
        modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        modelLayoutBinding.pImmutableSamplers = nullptr;*/

        std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {uboVPLayoutBinding};//, modelLayoutBinding};

        VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
        layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());  //number of binding infos
        layoutCreateInfo.pBindings = layoutBindings.data(); //pointer to binding info;

        if( vkCreateDescriptorSetLayout(device.device(), &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS){
            throw std::runtime_error("Failed to create descriptor set layout!");
        }

        //create texture sampler descriptor set layout;
        //texture binding info
        if (TEXTURE) {
            VkDescriptorSetLayoutBinding samplerLayoutBinding1 = {};
            samplerLayoutBinding1.binding = 0; //from set 1
            samplerLayoutBinding1.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding1.descriptorCount = 1;
            samplerLayoutBinding1.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            samplerLayoutBinding1.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutBinding samplerLayoutBinding2 = {};
            samplerLayoutBinding2.binding = 1; //from set 1
            samplerLayoutBinding2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding2.descriptorCount = 1;
            samplerLayoutBinding2.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            samplerLayoutBinding2.pImmutableSamplers = nullptr;

            std::array<VkDescriptorSetLayoutBinding, 2> samplerlayoutBindings = {samplerLayoutBinding1,
                                                                                 samplerLayoutBinding2};

            //create a descriptor set layout with given bindings for texture
            VkDescriptorSetLayoutCreateInfo texLayoutCreateInfo = {};
            texLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            texLayoutCreateInfo.bindingCount = 2;
            texLayoutCreateInfo.pBindings = samplerlayoutBindings.data();

            if( vkCreateDescriptorSetLayout(device.device(), &texLayoutCreateInfo, nullptr, &samplerSetLayout) != VK_SUCCESS){
                throw std::runtime_error("Failed to create sampler descriptor set layout!");
            }
        }
    }

    void NewVulkanApp::createPipeline(int flag) {
        pipelineConfig = VulkanPipeline::defaultPipelineConfigInfo(vulkanSwapChain->width(), vulkanSwapChain->height());
        pipelineConfig.renderPass = vulkanSwapChain->getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;

        if (flag==0){
            vulkanPipelines.push_back(std::move(std::make_unique<VulkanPipeline>(device,
                                                                                 "shaders/notextvert.spv",
                                                                                 "shaders/notextfrag.spv",
                                                                                 pipelineConfig)));
        }
        if (flag==1) {
            vulkanPipelines.push_back(std::move(std::make_unique<VulkanPipeline>(device,
                                                                                 "shaders/textvert.spv",
                                                                                 "shaders/textfrag.spv",
                                                                                 pipelineConfig)));
        }
        if (flag==2) {
            vulkanPipelines.push_back(std::move(std::make_unique<VulkanPipeline>(device,
                                                                                 "shaders/textvert.spv",
                                                                                 "shaders/textfrag.spv",
                                                                                 pipelineConfig)));
        }
        if( flag==3){
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
            pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
            vulkanPipelines.push_back(std::move(std::make_unique<VulkanPipeline>(device,
                                                                                 "shaders/notextvert.spv",
                                                                                 "shaders/notextfrag.spv",
                                                                                 pipelineConfig)));
        }
    }

    void NewVulkanApp::createUniformBuffers() {

        VkDeviceSize vpBufferSize = sizeof(UboVP); //size of all three variable - will offset to access it;
        //VkDeviceSize mDynBufferSize = modelUniformAlignment * MAX_OBJECTS; //size of all three variable - will offset to access it;


        vpUniformBuffer.resize(vulkanSwapChain->imageCount());
        vpUniformBufferMemory.resize(vulkanSwapChain->imageCount());

        //mDynUniformBuffer.resize(vulkanSwapChain.imageCount());
        //mDynUniformBufferMemory.resize(vulkanSwapChain.imageCount());

        for(size_t i=0; i<vulkanSwapChain->imageCount(); i++){
            device.createBuffer(vpBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                vpUniformBuffer[i], vpUniformBufferMemory[i]);

            //device.createBuffer(mDynBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                //VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                //mDynUniformBuffer[i], mDynUniformBufferMemory[i]);
        }
    }

    void NewVulkanApp::createDescriptorPool() {

        //how many descriptors, not descriptor sets.
        VkDescriptorPoolSize vpPoolSize = {};
        vpPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        vpPoolSize.descriptorCount = static_cast<uint32_t>(vpUniformBuffer.size());

        /*//dynamic pool size
        VkDescriptorPoolSize mDynPoolSize = {};
        mDynPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        mDynPoolSize.descriptorCount = static_cast<uint32_t>(mDynUniformBuffer.size());*/

        std::vector<VkDescriptorPoolSize> descriptorPoolSizes = {vpPoolSize}; //, mDynPoolSize}; (for dynamic buffer objects

        VkDescriptorPoolCreateInfo poolCreateInfo = {}; //data to create descriptor pool
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.maxSets = static_cast<uint32_t>(vulkanSwapChain->imageCount());
        poolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size()); //number of poolSize structs we are passing in.
        poolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

        if(vkCreateDescriptorPool(device.device(), &poolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS){
            throw std::runtime_error("failed to create a descriptor pool");
        }

        //create sampler descriptor pool
        //texture sampler pool
        if(TEXTURE) {
            VkDescriptorPoolSize samplerPoolSize = {};
            samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerPoolSize.descriptorCount = MAX_OBJECTS; //we are assuming each object will have one texture.

            VkDescriptorPoolCreateInfo samplerPoolCreateInfo = {};
            samplerPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            samplerPoolCreateInfo.maxSets = MAX_OBJECTS;
            samplerPoolCreateInfo.poolSizeCount = 1;
            samplerPoolCreateInfo.pPoolSizes = &samplerPoolSize;

            if (vkCreateDescriptorPool(device.device(), &samplerPoolCreateInfo, nullptr, &samplerDescriptorPool) !=
                VK_SUCCESS) {
                throw std::runtime_error("failed to create a sampler descriptor pool");
            }
        }

        //ImGui Descriptor Pool creation
        VkDescriptorPoolSize imGuiPoolSizes[] =
                {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
                };
        VkDescriptorPoolCreateInfo ImGuiPoolInfo = {};
        ImGuiPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        ImGuiPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        ImGuiPoolInfo.maxSets = 1000 * IM_ARRAYSIZE(imGuiPoolSizes);
        ImGuiPoolInfo.poolSizeCount = (uint32_t)IM_ARRAYSIZE(imGuiPoolSizes);
        ImGuiPoolInfo.pPoolSizes = imGuiPoolSizes;

        VkResult err = vkCreateDescriptorPool(device.device(), &ImGuiPoolInfo, nullptr, &ImGuiDescriptorPool);
        check_vk_result(err);
    }

    void NewVulkanApp::createDescriptorSets() {
        descriptorSets.resize(vulkanSwapChain->imageCount()); //resize the descriptor set so there is one for every buffer

        std::vector<VkDescriptorSetLayout> setLayouts(vulkanSwapChain->imageCount(), descriptorSetLayout);

        VkDescriptorSetAllocateInfo setAllocInfo = {}; //descriptor set allocation info
        setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocInfo.descriptorPool = descriptorPool;
        setAllocInfo.descriptorSetCount = static_cast<uint32_t>(vulkanSwapChain->imageCount());
        setAllocInfo.pSetLayouts = setLayouts.data(); //one to one relationship between the descriptorsetLayout and sets. layout to allocate sets.

        if (vkAllocateDescriptorSets(device.device(), &setAllocInfo, descriptorSets.data()) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate descriptor sets");
        }

        for(size_t i=0; i<vulkanSwapChain->imageCount(); i++){
            VkDescriptorBufferInfo uboVPBufferInfo = {};
            uboVPBufferInfo.buffer = vpUniformBuffer[i];
            uboVPBufferInfo.offset = 0; //if we only want to bind half of the descriptor for example. we want ot start from zero to bind everything
            uboVPBufferInfo.range = sizeof(UboVP); //size of data chunk;

            VkWriteDescriptorSet uboVPSetWrite = {}; //data about connection between buffer and binding. the descriptor set will be bound to the buffer using the following info.
            uboVPSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uboVPSetWrite.dstSet = descriptorSets[i]; // destination set
            uboVPSetWrite.dstBinding = 0; //which binding in our descriptor set are we updating.
            uboVPSetWrite.dstArrayElement = 0; //index in array to update.
            uboVPSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboVPSetWrite.descriptorCount = 1; //amount to update
            uboVPSetWrite.pBufferInfo = &uboVPBufferInfo; //info about buffer info.

            /*//dynamic buffer binding info
            VkDescriptorBufferInfo mDynBufferInfo = {};
            mDynBufferInfo.buffer = mDynUniformBuffer[i];
            mDynBufferInfo.offset = 0;
            mDynBufferInfo.range = modelUniformAlignment;

            VkWriteDescriptorSet mDynSetWrite = {};
            mDynSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            mDynSetWrite.dstSet = descriptorSets[i];
            mDynSetWrite.dstBinding = 1;
            mDynSetWrite.dstArrayElement = 0;
            mDynSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
            mDynSetWrite.descriptorCount = 1;
            mDynSetWrite.pBufferInfo = &mDynBufferInfo; */

            std::vector<VkWriteDescriptorSet> setWrites = {uboVPSetWrite}; //, mDynSetWrite};

            //update the descriptor set with the new buffer/binding info
            vkUpdateDescriptorSets(device.device(), static_cast<uint32_t>(setWrites.size()), setWrites.data(), 0, nullptr);
        }
    }

    void NewVulkanApp::createCommandBuffers() {
        commandBuffers.resize(vulkanSwapChain->imageCount());

        VkCommandBufferAllocateInfo allocateInfo{};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //submitted directly to queue. can't be called by other command buffers.
        allocateInfo.commandPool = device.getCommandPool();
        allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device.device(), &allocateInfo, commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers");
        }

    }

    void NewVulkanApp::recordCommand(uint32_t imageIndex){

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //buffer can be re-submitted when it has already been submitted on the queue and is awaiting for execution.

        //info on how to begin the render pass. only used for graphical application
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = vulkanSwapChain->getRenderPass();
        renderPassInfo.renderArea.offset = {0,0};
        renderPassInfo.renderArea.extent = vulkanSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.005f, 0.005f, 0.005f, 1.0f}; //clear value for attachment 0 (colour)
        clearValues[1].depthStencil = {1.0f, 0}; //clear value for attachment 1 (depth)
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        renderPassInfo.framebuffer = vulkanSwapChain->getFrameBuffer(imageIndex);

        //start recording command to command buffer!
        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording");
        }

        vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE); //starts by clearing the image with the clearValue

        //vkCmdSetViewport(commandBuffers[i],0,1,&pipelineConfig.viewport); //flip the viewport along the y axis.

        for (int o = 0; o < objectToRender.size(); o++) {

            //std::cout << "size of objectToRender List: " << objectToRender.size() << " with object "<<objectToRender[o] << std::endl;
            if(currPipeline == 1){
                vulkanPipelines[3]->bind(commandBuffers[imageIndex]);
            }else{
                vulkanPipelines[pipelineToUse[objectToRender[o]]]->bind(commandBuffers[imageIndex]);
            }
            modelList[objectToRender[o]]->bind(commandBuffers[imageIndex]);
            modelList[objectToRender[o]]->bindIndexed(commandBuffers[imageIndex]);

            //uint32_t dynamicOffset = static_cast<uint32_t>(modelUniformAlignment) * m;
            vkCmdPushConstants(commandBuffers[imageIndex],
                               pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT,
                               0,
                               sizeof(PushObject),
                               modelList[objectToRender[o]]->getModel());


            std::array<VkDescriptorSet, 2> descriptorSetGroup = {descriptorSets[imageIndex],
                                                                 samplerDescriptorSets[modelList[objectToRender[o]]->getTexID()]};

            vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipelineLayout, 0, 2, descriptorSetGroup.data(),
                                    0,
                                    nullptr); //we have no dynamic offset yet (only using uniform buffer);

                                    modelList[objectToRender[o]]->drawIndexed(commandBuffers[imageIndex]);

        }

        vulkanPipelines[0]->bind(commandBuffers[imageIndex]);
        //scene.bind(commandBuffers[imageIndex],pipelineLayout,descriptorSets[imageIndex],samplerDescriptorSets);

        //std::cout<<draw_data->Valid<<std::endl;
        ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffers[imageIndex]);

        vkCmdEndRenderPass(commandBuffers[imageIndex]);

        // ends the recording of the command buffer!
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }

    }

    void NewVulkanApp::updateUniformBuffers(uint32_t imageIndex) {
        //copy vp data
        void *data;
        vkMapMemory(device.device(), vpUniformBufferMemory[imageIndex], 0, sizeof(UboVP), 0, &data);
        memcpy(data, &uboVP, sizeof(UboVP));
        vkUnmapMemory(device.device(), vpUniformBufferMemory[imageIndex]);


        /** kept for futur reference. Dynamic Buffer memcpy
        //copy model data
        for (size_t i=0; i<modelList.size(); i++){ //here we don't want to use maxobjects since its useless to use memory for objects that dont exist.
            pushObject* thisModel = (pushObject*)((uint64_t)modelTransferSpace + (i * modelUniformAlignment)); //modeltransferspace has just been altered
            *thisModel = modelList[i]->getUbo();
        }
        //map the list of model data
        vkMapMemory(device.device(), mDynUniformBufferMemory[imageIndex], 0, modelUniformAlignment * modelList.size(), 0, &data); //reuse data since it has been freed.
        memcpy(data, modelTransferSpace, modelUniformAlignment * modelList.size());
        vkUnmapMemory(device.device(), mDynUniformBufferMemory[imageIndex]);
         **/
    }

    void NewVulkanApp::allocateDynamicBufferTransferSpace() {
        //find allignment based on the ubo
        //modelUniformAlignment = (sizeof(pushObject) + device.properties.limits.minUniformBufferOffsetAlignment -1) & ~(device.properties.limits.minUniformBufferOffsetAlignment - 1);

        //alocate a certain amount of memory for objects. we have to decide how many objects can be rendered (MAX)
        //modelTransferSpace = (pushObject*)_mm_malloc(modelUniformAlignment * MAX_OBJECTS, modelUniformAlignment);
    }

    void NewVulkanApp::drawFrame(){
        uint32_t imageIndex;
        auto result = vulkanSwapChain->acquireNextImage(&imageIndex);

        if(result == VK_ERROR_OUT_OF_DATE_KHR){
            recreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){
            throw std::runtime_error( "failed to acquire swap chain image");
        }

        updateUniformBuffers(imageIndex);
        recordCommand(imageIndex);
        result = vulkanSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vulkanWindow.wasWindowResized()){
            vulkanWindow.resetWindowResizedFlag();
            recreateSwapchain();
            return;
        }

        if (result != VK_SUCCESS) {
            throw std::runtime_error( "failed to present swap chain image");
        }
    }

    std::vector<Vertex> NewVulkanApp::getNormals(std::vector<Vertex> shape) {
        std::vector<Vertex> normalList;

        for(int i=0; i<shape.size(); i++){
            Vertex v1;
            v1.position = shape[i].position;
            v1.colour = glm::vec3(1.0f,1.0f,1.0f);
            Vertex v2;
            v2.position = shape[i].position + shape[i].norm;
            v2.colour = glm::vec3(1.0f,1.0f,1.0f);
            normalList.push_back(v1);
            normalList.push_back(v2);
        }
        return normalList;
    }

    glm::vec3 NewVulkanApp::getVectorFromMouse(double xPos, double yPos) {

        //std::cout<<"["<<xPos<<","<<yPos<<"]"<<std::endl;

        float width = (float)vulkanSwapChain->getSwapChainExtent().width;
        float height = (float)vulkanSwapChain->getSwapChainExtent().height;
        float centerX = width/2.0f;
        float centerY = height/2.0f;
        float radius = glm::min(width,height)/2*FIT;

        glm::vec3 pt(0.0f);
        pt.x = ((float)xPos-centerX);
        pt.y = (centerY - (float)yPos);
        float ptLen = glm::length(pt);

        if(ptLen>radius){
            pt = glm::normalize(pt);
        } else {
            pt.z = glm::sqrt(radius*radius - ptLen*ptLen);
            pt = glm::normalize(pt);
        }

        return pt;
    }

    void NewVulkanApp::updateModels() {
        //float sec = glfwGetTime();
        double dot;
        double angle;
        glm::vec3 Axis;
        glm::mat4 R = glm::mat4(1.0f);
        glm::mat4 VInv = glm::inverse(uboVP.V);
        glm::mat4 V = uboVP.V;
        glm::mat4 result;


        if(MPRESS_R && MFLAG_R){
            MFLAG_R = false;
            //view = glm::mat4(uboVP.V);
            double xPosInit,yPosInit;
            glfwGetCursorPos(vulkanWindow.getWindow(),&xPosInit,&yPosInit);
            glm::vec4 temp1 = VInv * glm::vec4(getVectorFromMouse(xPosInit,yPosInit),0.0f);
            mouseInit = normalize(glm::vec3(temp1.x,temp1.y,temp1.z));
        }

        if(MPRESS_R){
            double xPosCurr,yPosCurr;
            //glm::mat4 Mmodel = modelList[objectToRender[0]]->getModel()->M;
            glfwGetCursorPos(vulkanWindow.getWindow(),&xPosCurr,&yPosCurr);

            glm::vec4 temp = VInv * glm::vec4(getVectorFromMouse(xPosCurr,yPosCurr),0.0f);
            mouseCurr = glm::normalize(glm::vec3(temp.x,temp.y,temp.z));
            //std::cout<<"mouseCurr: "<<mouseCurr.x<<" "<<mouseCurr.y<<" "<<mouseCurr.z<<std::endl;

            //std::cout<<glm::length(glm::vec3(mouseCurr - mouseInit))<<std::endl;

            if(glm::length(glm::vec3(mouseCurr - mouseInit)) < 1E-05){
                //std::cout<<"do nothing"<<std::endl;
            } else {

                Axis = glm::cross(mouseInit, mouseCurr);

                dot = glm::dot(mouseInit, mouseCurr);
                if(dot>1) {dot = 1;}
                angle = glm::acos(dot);

                //std::cout<<"mouse curr: ["<<mouseCurr.x<<","<<mouseCurr.y<<","<<mouseCurr.z<<"], "<<"mouse init: ["<<mouseInit.x<<","<<mouseInit.y<<","<<mouseInit.z<<"], "<<angle<<std::endl;

                //modelList[objectToRender[0]]->setModel(R * Mmodel);
                //modelList[modelList.size()-1]->setModel(R * Mmodel);
                //glm::mat4 tempV = glm::translate(glm::mat4(1.0f),glm::vec3(glm::inverse(uboVP.V)[3][0],glm::inverse(uboVP.V)[3][1],glm::inverse(uboVP.V)[3][2]));

                //uboVP.V = uboVP.V * glm::inverse(R);
                uboVP.V = uboVP.V * glm::rotate(glm::mat4(1.0f), (float)angle, Axis);
            }
        }

        if(MPRESS_L && MFLAG_L){
            MFLAG_L = false;
            //view = glm::mat4(uboVP.V);
            double xPosInit,yPosInit;
            glfwGetCursorPos(vulkanWindow.getWindow(),&xPosInit,&yPosInit);
            glm::vec4 temp1 = VInv * glm::vec4(getVectorFromMouse(xPosInit,yPosInit),0.0f);
            mouseInit = normalize(glm::vec3(temp1.x,temp1.y,temp1.z));
        }

        if(MPRESS_L){
            double xPosCurr,yPosCurr;
            glm::mat4 Mmodel = modelList[objectToRender[0]]->getModel()->M;
            glfwGetCursorPos(vulkanWindow.getWindow(),&xPosCurr,&yPosCurr);

            glm::vec4 temp = VInv * glm::vec4(getVectorFromMouse(xPosCurr,yPosCurr),0.0f);
            mouseCurr = glm::normalize(glm::vec3(temp.x,temp.y,temp.z));
            //std::cout<<"mouseCurr: "<<mouseCurr.x<<" "<<mouseCurr.y<<" "<<mouseCurr.z<<std::endl;

            //std::cout<<glm::length(glm::vec3(mouseCurr - mouseInit))<<std::endl;

            if(glm::length(glm::vec3(mouseCurr - mouseInit)) < 1E-05){
                //std::cout<<"do nothing"<<std::endl;
            } else {

                Axis = glm::cross(mouseInit, mouseCurr);

                dot = glm::dot(mouseInit, mouseCurr);
                if(dot>1) {dot = 1;}
                angle = glm::acos(dot) * GAIN;

                R = glm::rotate(glm::mat4(1.0f), (float)angle, Axis);
                //std::cout<<"mouse curr: ["<<mouseCurr.x<<","<<mouseCurr.y<<","<<mouseCurr.z<<"], "<<"mouse init: ["<<mouseInit.x<<","<<mouseInit.y<<","<<mouseInit.z<<"], "<<angle<<std::endl;

                modelList[objectToRender[0]]->setModel(R * Mmodel);
                //modelList[modelList.size()-1]->setModel(R * Mmodel);
                //glm::mat4 tempV = glm::translate(glm::mat4(1.0f),glm::vec3(glm::inverse(uboVP.V)[3][0],glm::inverse(uboVP.V)[3][1],glm::inverse(uboVP.V)[3][2]));

                //uboVP.V = uboVP.V * glm::inverse(R);
                //uboVP.V = uboVP.V * glm::rotate(glm::mat4(1.0f), (float)angle, Axis);
                mouseInit = mouseCurr;
            }
        }

        //std::cout<<"["<<xPos<<","<<yPos<<"]"<<std::endl;

        ///xPos/=vulkanSwapChain->getSwapChainExtent().width;
        //yPos/=vulkanSwapChain->getSwapChainExtent().height;
        //xPos = (xPos*2)-1;
        //yPos = -(yPos*2)+1;

        //uboVP.lightPos = glm::vec4(3.0f,3.0f,4.0f,1.0f);
        uboVP.P = glm::perspective(glm::radians(scene.getCameras()[currCamera].camFovy - scroll), (float) vulkanSwapChain->width()/(float)vulkanSwapChain->height(),0.1f, 100.0f);

        float increment =1.0f;
        if (COM){
            increment = 0.3f;
        }

        if (SHIFT){
            if (RIGHT) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(increment), glm::vec3(0.0f, 0.0f, 1.0f)) * M1;
                //uboVP.lightPos = glm::vec4((float)xPos*5.0f,(float)yPos*1.0f,-2.0f,1.0f);
            }
            if (LEFT) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(-increment), glm::vec3(0.0f, 0.0f, 1.0f)) * M1;
                //uboVP.lightPos = glm::vec4((float)xPos*5.0f,(float)yPos*1.0f,-2.0f,1.0f);
            }
        } else {
            if (UP_PRESS) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(-increment), glm::vec3(1.0f, 0.0f, 0.0f)) * M1;
            }
            if (DOWN) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(increment), glm::vec3(1.0f, 0.0f, 0.0f)) * M1;
            }
            if (RIGHT) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(increment), glm::vec3(0.0f, 1.0f, 0.0f)) * M1;
            }
            if (LEFT) {
                M1 = glm::rotate(glm::mat4(1.0f), glm::radians(-increment), glm::vec3(0.0f, 1.0f, 0.0f)) * M1;
            }
        }

        //modelList[0]->setModel(M1);
        //modelList[1]->setModel(Scale);
        //M2 =  M2 * glm::rotate(glm::mat4(1.0f), glm::radians(sec * 90.0f),glm::vec3(0.0f,1.0f,0.0f));
        //modelList[0]->setModel(M2);
        //modelList[1]->setModel(Scale*M2);

    }

    void NewVulkanApp::createPushConstantRange() {
        //define push constand values (no 'create' needed);
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;       //offset into given data
        pushConstantRange.size = sizeof(PushObject);
    }

    void NewVulkanApp::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        {

            if (key == GLFW_KEY_UP && action == GLFW_PRESS){
                UP_PRESS = true;
            } else if (key == GLFW_KEY_UP && action == GLFW_RELEASE) {
                UP_PRESS = false;
            }

            if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
                DOWN = true;
            } else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE) {
                DOWN = false;
            }

            if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
                RIGHT = true;
            } else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE) {
                RIGHT = false;
            }

            if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
                LEFT = true;
            } else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
                LEFT = false;
            }

            if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS){
                SHIFT = true;
            } else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
                SHIFT = false;
            }

            if (key == GLFW_KEY_LEFT_SUPER && action == GLFW_PRESS){
                COM = true;
            } else if (key == GLFW_KEY_LEFT_SUPER && action == GLFW_RELEASE) {
                COM = false;
            }
        }
    }

    void NewVulkanApp::recreateSwapchain() {
        auto extent = vulkanWindow.getExtent();
        while(extent.width == 0 || extent.height == 0){
            extent = vulkanWindow.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(device.device());
        vulkanSwapChain.reset();
        vulkanSwapChain = std::make_unique<VulkanSwapChain>(device,extent);
        uboVP.P = glm::perspective(glm::radians(65.0f), (float) vulkanSwapChain->width()/(float)vulkanSwapChain->height(),0.1f, 100.0f);
        vulkanPipelines.clear();
        createPipeline(0);
        createPipeline(1);
        createPipeline(2);
        createPipeline(3);
    }

    stbi_uc *NewVulkanApp::loadTextureFile(std::string filename, int * width, int * height, VkDeviceSize* imageSize) {
        //number of channels the image usage
        int channels;

        //load pixel data for image
        std::string fileLoc = "textures/" + filename;
        stbi_uc * image = stbi_load(fileLoc.c_str(), width, height, &channels, STBI_rgb_alpha);

        if(image == NULL){
            throw std::runtime_error("failed to load texture file: " + filename );
        }

        *imageSize = (*width) * (*height) * 4; //4 channels

        return image;
    }

    int NewVulkanApp::createTextureImage(std::string filename, std::string norm) {
        int width,height;
        VkDeviceSize imageSize;
        stbi_uc* imageData = loadTextureFile(filename, &width, &height, &imageSize);

        //create staging buffer to hold image data ready to send to device
        VkBuffer imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        device.createBuffer(imageSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ,
                            imageStagingBuffer, imageStagingBufferMemory);

        //copy image data to staging buffer;
        void *data;
        vkMapMemory(device.device(), imageStagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, imageData, static_cast<size_t>(imageSize));
        vkUnmapMemory(device.device(), imageStagingBufferMemory);

        //create Vk_image to hold file texture
        VkImage texImage; //VkImage and VkBuffer are both represented by a chunk of data (VkDeviceMemory)
        VkDeviceMemory texMemory;


        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        device.createImageWithInfo(imageInfo,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texImage, texMemory);

        //transition image to be dst for copy operation
        device.transitionImageLayout(device.graphicsQueue(), device.getCommandPool(), texImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        device.copyBufferToImage(imageStagingBuffer, texImage, width, height, 1);
        //transition image to be shader readable for shader usage
        device.transitionImageLayout(device.graphicsQueue(), device.getCommandPool(), texImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        //add texture data to the texture list;
        textureImages.push_back(texImage);
        textureImageMemory.push_back(texMemory);

        vkDestroyBuffer(device.device(),imageStagingBuffer, nullptr);
        vkFreeMemory(device.device(),imageStagingBufferMemory, nullptr);

        //norms
        if(!norm.empty()) {
            stbi_uc *imageDataNorm = loadTextureFile(norm, &width, &height, &imageSize);

            VkBuffer normImageStagingBuffer;
            VkDeviceMemory normImageStagingBufferMemory;

            device.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                normImageStagingBuffer, normImageStagingBufferMemory);

            vkMapMemory(device.device(), normImageStagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, imageDataNorm, static_cast<size_t>(imageSize));
            vkUnmapMemory(device.device(), normImageStagingBufferMemory);

            stbi_image_free(imageDataNorm);

            //free original image data
            stbi_image_free(imageData);

            VkImage texImageNorm; //VkImage and VkBuffer are both represented by a chunk of data (VkDeviceMemory)
            VkDeviceMemory texMemoryNorm;
            imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; //make sure the textures are in normalized coordinates
            device.createImageWithInfo(imageInfo,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texImageNorm, texMemoryNorm);

            device.transitionImageLayout(device.graphicsQueue(), device.getCommandPool(), texImageNorm, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            device.copyBufferToImage(normImageStagingBuffer, texImageNorm, width, height, 1);

            device.transitionImageLayout(device.graphicsQueue(), device.getCommandPool(), texImageNorm, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            textureNormImages.push_back(texImageNorm);
            textureNormImageMemory.push_back(texMemoryNorm);

            vkDestroyBuffer(device.device(),normImageStagingBuffer, nullptr);
            vkFreeMemory(device.device(),normImageStagingBufferMemory, nullptr);

        }

        return textureImages.size()-1;
    }

    int NewVulkanApp::createTexture(std::string fileName, std::string norm) {

        if (!TEXTURE || fileName.empty()) {
            return -1;
        }
        //create texture and get its index
        int textureImageLoc = createTextureImage(fileName,norm);

        //create image view and add to list
        VkImageView imageView = vulkanSwapChain->createImageView(device.device(),textureImages[textureImageLoc],VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        textureImageViews.push_back(imageView);

        int descriptorLoc;

        if(!norm.empty()) {
            VkImageView normImageView = vulkanSwapChain->createImageView(device.device(),
                                                                         textureNormImages[textureImageLoc],
                                                                         VK_FORMAT_R8G8B8A8_UNORM,
                                                                         VK_IMAGE_ASPECT_COLOR_BIT);
            textureNormImageViews.push_back(normImageView);
            descriptorLoc = createTextureAndNormDescriptor(imageView, normImageView);
        } else {
            //TODO: create texture descriptor set here.
            descriptorLoc = createTextureDescriptor(imageView);
        }

        return descriptorLoc;
    }

    void NewVulkanApp::createTextureSampler() {
        //Sampler creation info
        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR; //how to render when image gets magnified on screen.
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR; //how to render when image gets minified on screen.
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; //if we go outside of the texture, how we render? by wrapping around and repeating. (U = x direction, V = y, W = z);
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; //border beyond tex. only works for BORDER_CLAMP
        samplerCreateInfo.unnormalizedCoordinates = VK_FALSE; //means it is normalized (between 0-1) coordinates.
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; //as we move away the rate at which it fades away is linear. we won't really see it happen since we have no implemented it yet
        samplerCreateInfo.mipLodBias = 0.0f; //no offset to how the mipmaps are handled.
        samplerCreateInfo.minLod = 0.0f; //min level of detail of mipmap;
        samplerCreateInfo.maxLod = 0.0f; //max level of detail of mipmap;
        samplerCreateInfo.anisotropyEnable = VK_TRUE; //blends pixels when they are stretched due to different angle of views to the texture.
        samplerCreateInfo.maxAnisotropy = 16;

        if(vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &texSampler) != VK_SUCCESS){
            throw std::runtime_error("failed to create sampler");
        }
    }

    int NewVulkanApp::createTextureDescriptor(VkImageView texImage) {
        VkDescriptorSet descriptorSet;

        //descriptor set allocation info
        VkDescriptorSetAllocateInfo setAllocateInfo = {};
        setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorPool = samplerDescriptorPool;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts = &samplerSetLayout;

        //allocate Descriptor Set
        if(vkAllocateDescriptorSets(device.device(),&setAllocateInfo,&descriptorSet) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate texture descriptor set");
        }

        //texture image info
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //what is the image layout when it is in use.
        imageInfo.imageView = texImage; //image to bind to set
        imageInfo.sampler = texSampler;

        //descriptor right info
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        //update the new descriptorSet
        vkUpdateDescriptorSets(device.device(), 1, &descriptorWrite, 0, nullptr); //update the descriptor set with the new info

        //add descriptor set to list
        samplerDescriptorSets.push_back(descriptorSet);
        return samplerDescriptorSets.size()-1; //return the location of the descriptor set
    }

    int NewVulkanApp::createTextureAndNormDescriptor(VkImageView texImage, VkImageView normImage) {
        VkDescriptorSet descriptorSet;

        //descriptor set allocation info
        VkDescriptorSetAllocateInfo setAllocateInfo = {};
        setAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorPool = samplerDescriptorPool;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts = &samplerSetLayout;

        //allocate Descriptor Set
        if(vkAllocateDescriptorSets(device.device(),&setAllocateInfo,&descriptorSet) != VK_SUCCESS){
            throw std::runtime_error("failed to allocate texture descriptor set");
        }

        //texture image info
        VkDescriptorImageInfo imageInfo = {};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //what is the image layout when it is in use.
        imageInfo.imageView = texImage; //image to bind to set
        imageInfo.sampler = texSampler;

        VkDescriptorImageInfo normImageInfo = {};
        normImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; //what is the image layout when it is in use.
        normImageInfo.imageView = normImage; //image to bind to set
        normImageInfo.sampler = texSampler;

        //descriptor right info
        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        VkWriteDescriptorSet normDescriptorWrite = {};
        normDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normDescriptorWrite.dstSet = descriptorSet;
        normDescriptorWrite.dstBinding = 1;
        normDescriptorWrite.dstArrayElement = 0;
        normDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        normDescriptorWrite.descriptorCount = 1;
        normDescriptorWrite.pImageInfo = &normImageInfo;

        std::array<VkWriteDescriptorSet, 2> descriptorSetWrites = {descriptorWrite, normDescriptorWrite};

        //update the new descriptorSet
        vkUpdateDescriptorSets(device.device(), 2, descriptorSetWrites.data(), 0, nullptr); //update the descriptor set with the new info

        //add descriptor set to list
        samplerDescriptorSets.push_back(descriptorSet);
        return samplerDescriptorSets.size()-1; //return the location of the descriptor set
    }

    void NewVulkanApp::mouse_callback(GLFWwindow *window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)){
            MPRESS_L = true;
        } else {
            MPRESS_L = false;
        }

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
            MFLAG_L = true;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow)){
            MPRESS_R = true;
        } else {
            MPRESS_R = false;
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
            MFLAG_R = true;
        }
    }

    void NewVulkanApp::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        //std::cout<<yoffset<<std::endl;
        scroll += 2.0f*yoffset;
    }


    void NewVulkanApp::init_imgui() {


        //this initializes the core structures of imgui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(vulkanWindow.getWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.physDevice();
        init_info.Device = device.device();
        init_info.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
        init_info.Queue = device.graphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = ImGuiDescriptorPool;
        init_info.Allocator = nullptr;
        init_info.MinImageCount = 2;
        init_info.ImageCount = vulkanSwapChain->MAX_FRAMES_IN_FLIGHT;
        init_info.CheckVkResultFn = check_vk_result;
        init_info.Subpass = 0;
        ImGui_ImplVulkan_Init(&init_info, vulkanSwapChain->getRenderPass());

        //execute a gpu command to upload imgui font textures
        // Upload Fonts
        {

            // Use any command queue
            VkCommandBuffer command_buffer = commandBuffers[0];

            err = vkResetCommandPool(device.device(), device.getCommandPool(), 0);
            check_vk_result(err);
            VkCommandBufferBeginInfo begin_info = {};
            begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(command_buffer, &begin_info);
            check_vk_result(err);

            ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

            err = vkEndCommandBuffer(command_buffer);
            check_vk_result(err);

            //err = vkDeviceWaitIdle(device.device());
            //check_vk_result(err);

            VkSubmitInfo end_info = {};
            end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            end_info.commandBufferCount = 1;
            end_info.pCommandBuffers = &command_buffer;

            err = vkQueueSubmit(device.graphicsQueue(), 1, &end_info, VK_NULL_HANDLE);
            check_vk_result(err);

            err = vkDeviceWaitIdle(device.device());
            check_vk_result(err);

            ImGui_ImplVulkan_DestroyFontUploadObjects();
        }
    }


    void NewVulkanApp::imGuiParametersSetup() {

        double time = glfwGetTime();
        static bool wireframeEnabled;
        static bool animLightEnabled;
        static bool floorEnabled = true;
        static glm::vec3 colour = modelList[objectToRender[0]]->getColour();
        float temp_x = 0;
        float temp_z = 0;

        //std::cout<<objectToRender[0]<<" is: "<<modelList[objectToRender[0]]->getColour().x<<std::endl;

        ImGui::Begin("My very own Vulkan Render Engine!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::SetWindowSize(ImVec2(350.0f,300.0f),0);

        ImGui::Text("Specular Highlight.");               // Display some text (you can use a format strings too)

        ImGui::SliderFloat("float", &modelList[objectToRender[0]]->getModel()->specular, 8.0f, 128.0f);            // Edit 1 float using a slider from 0.0f to 1.0f

        if(ImGui::SliderFloat3("float", &uboVP.lightPos.x, -10.0f, 10.0f)){
            animLightEnabled = false;
        }

        ImGui::Text("Colour.");
        if(ImGui::ColorEdit3("colour", (float*)&colour)){
            modelList[objectToRender[0]]->setColour(glm::vec4(colour[0],colour[1],colour[2],1.0f),device.graphicsQueue(),device.getCommandPool());
        } // Edit 3 floats representing a color

        //const char* items[] = objectfiles().data();

        static const char* current_item = NULL;


        ImGui::Text("import object");
        if (ImGui::BeginCombo("##combo1", current_item)) // The second parameter is the label previewed before opening the combo.
            {
            for (int n = 0; n < items.size(); n++)
            {
                bool is_selected = (current_item == items[n].c_str()); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(items[n].c_str(), is_selected)){
                    current_item = items[n].c_str();
                    objectToRender[0] = n + 1;
                    colour = modelList[objectToRender[0]]->getColour();
                    modelList.back()->setModel(modelList[objectToRender[0]]->getModel()->M);
                }
                if (is_selected){
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
            }
            ImGui::EndCombo();
            }

        static const char* current_texture = NULL;

        ImGui::Text("import Texture");
        if (ImGui::BeginCombo("##combo2", current_texture)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < texturesPath.size(); n++)
            {
                bool is_selected = (current_texture == texturesPath[n].c_str()); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(texturesPath[n].c_str(), is_selected)){
                    current_texture = texturesPath[n].c_str();
                    Cube c;
                    c.transform(glm::mat4(3.0f));
                    addModelAt(c,0,texturesPath[n] + ".jpg",texturesPath[n] + "UV.jpg");
                }
                if (is_selected){
                    ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Checkbox("Wireframe mode", &wireframeEnabled);

        if(wireframeEnabled){
            currPipeline = 1;
        } else {
            currPipeline = 0;
        }

        ImGui::Checkbox("Animate Light", &animLightEnabled);

        if(animLightEnabled){
            uboVP.lightPos.x = temp_x + 10.0f * glm::cos(time);
            //std::cout<<"light.x"<<uboVP.lightPos.x<<std::endl;
            uboVP.lightPos.z = temp_z + 10.0f * glm::sin(time);
            //std::cout<<"light.x"<<uboVP.lightPos.x<<std::endl;
        }

        ImGui::Checkbox("Add floor", &floorEnabled);

        if(floorEnabled && objectToRender.size()==1){
            objectToRender.push_back(modelList.size()-1);
        } else if(floorEnabled){
            //do nothing
        }
        else{
            objectToRender.resize(1);
        }


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    std::vector<std::string> NewVulkanApp::objectfiles() {
        std::vector<std::string> listObjects;

        std::string path = "objects";
        for (const auto & entry : std::filesystem::directory_iterator(path)){
            if (entry.path().filename().extension() == ".obj" && entry.path().filename().c_str()[0] != '.'){
                listObjects.emplace_back(entry.path().stem().generic_string().c_str());
            }
        }

        return listObjects;
    }

    std::vector<std::string> NewVulkanApp::textureFiles() {
        std::vector<std::string> listObjects;

        std::string path = "textures";
        for (const auto & entry : std::filesystem::directory_iterator(path)){
            if (entry.path().filename().extension() == ".jpg" && entry.path().filename().c_str()[0] != '.' && entry.path().filename().generic_string().find("UV") == -1){
                listObjects.emplace_back(entry.path().stem().generic_string().c_str());
            }
        }

        return listObjects;
    }

}
