//
// Created by Hamza Lahmimsi on 2021-08-23.
//

#ifndef ARCBALL_CAMERA_H
#define ARCBALL_CAMERA_H

//glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"

class Camera{
public:
    struct Light{
        std::string name;
        glm::vec3 lightPos;
        glm::vec3 colour;
        float intensity;
    };
    //from="6 10 15" to="0 0 0" up="0 1 0" fovy="45" width="512" height="512"
    Camera(glm::vec3 from, glm::vec3 to, glm::vec3 up, float fovy, float width, float height){
        camMatView = glm::lookAt(glm::vec3(from), glm::vec3(to), glm::vec3(up));
        camMatProj = glm::perspective(glm::radians(fovy), width/height,0.1f, 100.0f);
        position = glm::vec3(from);
        camFovy = fovy;
    };

    Camera(){
        camMatView = glm::lookAt(glm::vec3(0.0f,1.5f,4.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
        camMatProj = glm::perspective(glm::radians(45.0f), 1200.0f/900.0f,0.1f, 100.0f);
    }
    glm::mat4 camMatView;
    glm::mat4 camMatProj;
    glm::vec3 position;
    float camFovy;
private:
};

#endif //ARCBALL_CAMERA_H
