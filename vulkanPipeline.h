//
// Created by Hamza Lahmimsi on 2021-02-19.
//

#ifndef VULKANTESTING_VULKANPIPELINE_H
#define VULKANTESTING_VULKANPIPELINE_H

#include "vulkanDevice.h"
#include "vulkanModel.h"

#include <string>
#include <vector>

namespace hva{

    struct PipelineConfigInfo{ // all of these need to be initialized. this is done in the defaultconfig function
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        uint32_t subpass = 0;
    };
    class VulkanPipeline{
    public:
        VulkanPipeline(VulkanDevice &device, const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);
        ~VulkanPipeline();
        VulkanPipeline(const VulkanPipeline&) = delete;
        VulkanPipeline& operator=(const VulkanPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        VkPipeline getPipeline(){ return graphicsPipeline; }

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    private:
        static std::vector<char> readFile(const std::string &filePath);

        void createGraphicsPipeline(const std::string &vertFilePath, const std::string &fragFilePath, const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        VulkanDevice &device;
        VkPipeline graphicsPipeline;
        VkShaderModule vertModule;
        VkShaderModule fragModule;
    };

}

#endif //VULKANTESTING_VULKANPIPELINE_H
