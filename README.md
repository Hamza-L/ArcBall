## [ArcBall](https://github.com/Hamza-L/ArcBall)
Here is the ArcBall UI allowing the user to move about an object in 3D space using only the mouse. This project builds on the VulkanEngine repo, where I test various Vulkan features and implement my own pseudo-engine.

<img src="https://github.com/Hamza-L/ArcBall/blob/master/Arcball.gif?raw=true" alt="ArcBall" width="1100"/>

<img src="https://github.com/Hamza-L/ArcBall/blob/master/ObjImporter.png?raw=true" alt="ArcBall" width="1100"/>

<img src="https://github.com/Hamza-L/ArcBall/blob/master/bunny.gif?raw=true" alt="ArcBall" width="1100"/>



### Features!* 

1. .Obj file importer.
1. 3D & 2D geometry
1. Texture from files
1. UV Normal textures form files
1. Scene graph, allowing objects to become part of a parent objects, sharing on local coordinate
1. specular highlight (bling-phong lighting)
1. User input
1. Scroll to zoom! makes use of perspective zoom rather than move the camera.

### How it works!
This implementation builds upon the **Ken Shoemake**'s paper _ARCBALL: A User Interface for Specifying
Three-Dimensional Orientation Using a Mouse_. Mouse coordinates are converted
vectors form the center of a fictional unit sphere. The starting mouse position and the ending mouse position allow us to establish an axis or rotation. Matrix accumulation allows us the compute rotation at very low cost.

![ArcInterpretation](kenShoemaker.png)

_Shoemake, K. (1992). ARCBALL: a user interface for specifying three-dimensional orientation using a mouse._

### Dependencies:
- glfw (window)
- Vulkan (graphics api)
- glm (math library)
- stb_image.h (for importing images: https://github.com/nothings/stb/blob/master/stb_image.h)

*_This repo includes features from the VulkanTesting repo, my very own engine._
