#!/bin/sh

#  compile.sh
#  VulkantTesting
#
#  Created by Hamza Lahmimsi on 2021-02-13.
#  

/Users/hamzalah/VulkanSDK/macOS/bin/glslc shader.vert -o vert.spv
/Users/hamzalah/VulkanSDK/macOS/bin/glslc shader.frag -o frag.spv
/Users/hamzalah/VulkanSDK/macOS/bin/glslc textureShader.vert -o textvert.spv
/Users/hamzalah/VulkanSDK/macOS/bin/glslc textureShader.frag -o textfrag.spv
