//
// Created by Hamza Lahmimsi on 2021-08-23.
//

#ifndef ARCBALL_SCENE_H
#define ARCBALL_SCENE_H

#include "shapes/Node.h"
#include "shapes/Cube.h"
#include "shapes/Icosahedron.h"
#include "shapes/ObjImporter.h"
#include "shapes/CheckerPlane.h"
#include "Camera.h"

#include <iostream>
#include <fstream>
#include <string>

class Scene : public Node {
public:
    Scene();
    Scene(std::string sceneName, float width, float height);
    void addCamera(glm::vec3 from, glm::vec3 to, glm::vec3 up, float fovy, float width, float height){
        cameras.push_back(Camera(from, to, up, fovy, width, height));
    };
    void addLight(std::string name, glm::vec3 position, glm::vec3 colour, float intensity){
        Camera::Light l;
        l.name = name;
        l.lightPos = position;
        l.colour = colour;
        l.intensity = intensity;

        lights.push_back(l);
    };
    std::vector<Node> getChildren(){return children;};
    std::vector<Camera::Light> getLights(){return lights;};
    std::vector<Camera> getCameras(){return cameras;};
private:
    Camera::Light getLightFromXML(std::string data);
    Cube getCubeFromXML(std::string data);
    Camera getCameraFromXML(std::string data);
    Node getNodeFromXML(std::string data);
    std::string getNodeStringParameter(std::string parameter, std::string data);
    float getNodeFloatParamter(std::string parameter, std::string data);
    glm::vec3 getNodeVec3Paramter(std::string parameter, std::string data);
    std::vector<std::string> getXMLlines(std::string data);
    std::vector<Camera> cameras{};
    std::vector<Camera::Light> lights{};
    glm::vec3 ambientLight = glm::vec3(0.02f,0.02f,0.02f);
    float width,height;
};


#endif //ARCBALL_SCENE_H
