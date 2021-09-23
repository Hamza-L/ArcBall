//
// Created by Hamza Lahmimsi on 2021-08-23.
//

#include "Scene.h"

Scene::Scene(std::string sceneName, float width, float height) {
    std::ifstream myfile("Scenes/"+ sceneName + ".xml");
    std::string data((std::istreambuf_iterator<char>(myfile)),
                    std::istreambuf_iterator<char>());

    int index = 0;
    int innerIndex = 0;
    std::string currStr;

    std::vector<std::string> stack;

    //getline(myfile, data);
    //std::cout<<data<<std::endl;
    std::vector<std::string> listOfLines = getXMLlines(data);

    for(auto & line : listOfLines){
        if(line.find("light") != -1){
            int t = line.find("light");
            lights.push_back(getLightFromXML(line));
        } else if (line.find("box") != -1){
            Cube c = getCubeFromXML(line);
            addChild(c);
        } else if (line.find("camera") != -1){
            Camera c = getCameraFromXML(line);
            cameras.push_back(c);
        }
    }
    //getLightFromXML(data);

}

Scene::Scene() {

}

Camera::Light Scene::getLightFromXML(std::string data) {

    int index = 0;
    int innerIndex = 0;
    std::string currStr;
    Camera::Light l;


    while(index != data.size()-1){

        if(data[index] == '<' && data[index+1] != '/' && data[index+1] != '?' && data[index+1] != '!'){
            innerIndex = index+1;
            while(data[innerIndex] != ' ' && data[innerIndex] != '\t'){
                currStr.push_back(data[innerIndex]);
                innerIndex++;
            }

            //std::cout<<"CURRENT STRING IS: "<<currStr<<std::endl;

            if(currStr == "light"){
                innerIndex = 0;
                std::string name;
                std::string num;
                std::vector<float> position;
                std::vector<float> colour;
                float intensity;

                innerIndex = data.substr(index,data.size()).find("name")+6;
                while(data[innerIndex] != '\"' ){
                    name.push_back(data[innerIndex]);
                    innerIndex++;
                }

                innerIndex = data.substr(index,data.size()).find("position")+10;
                while(data[innerIndex] != '\"' ){
                    if(data[innerIndex] != ' '){
                        num.push_back(data[innerIndex]);
                    } else {
                        position.push_back(std::stof(num));
                        num.clear();
                    }
                    innerIndex++;
                }
                position.push_back(std::stof(num));
                num.clear();

                innerIndex = data.substr(index,data.size()).find("color")+7;
                while(data[innerIndex] != '\"' ){
                    if(data[innerIndex] != ' '){
                        num.push_back(data[innerIndex]);
                    } else {
                        colour.push_back(std::stof(num));
                        num.clear();
                    }
                    innerIndex++;
                }
                colour.push_back(std::stof(num));
                num.clear();

                innerIndex = data.substr(index,data.size()).find("intensity")+11;
                while(data[innerIndex] != '\"' ){
                    num.push_back(data[innerIndex]);
                    innerIndex++;
                }
                intensity = std::stof(num);
                num.clear();

                l.name = name;
                l.lightPos = glm::vec3(position[0],position[1],position[2]);
                l.colour = glm::vec3(colour[0], colour[1], colour[2]);
                l.intensity = intensity;

            }
            currStr.clear();
            index = innerIndex;
            index++;
        }
        index++;
    }


    return l;
}

std::vector<std::string> Scene::getXMLlines(std::string data) {
    std::vector<std::string> list;

    for(int i = 0; i<data.size(); i++){
        if(data[i] == '<' && data[i+1] != '/' && data[i+1] != '?' && data[i+1] != '!'){
            int j=i;
            std::string entry;
            while(j != data.size()){
                //std::cout<<data[j]<<std::endl;
                if(data[j] == '>'){
                    entry.push_back(data[j]);
                    list.push_back(entry);
                    break;
                } else {
                    entry.push_back(data[j]);
                    j++;
                }
            }
            i = j;
        }
    }
    return list;
}

Cube Scene::getCubeFromXML(std::string data) {

    int index = 0;
    int innerIndex = 0;
    std::string currStr;

    Cube cube;


    while(index != data.size()-1){

        if(data[index] == '<' && data[index+1] != '/' && data[index+1] != '?' && data[index+1] != '!'){

            innerIndex = index+1;
            while(data[innerIndex] != ' ' && data[innerIndex] != '\t'){
                currStr.push_back(data[innerIndex]);
                innerIndex++;
            }

            //std::cout<<"CURRENT STRING IS: "<<currStr<<std::endl;

            if(currStr == "box"){
                innerIndex = 0;
                std::string name;
                std::string num;
                std::vector<float> min;
                std::vector<float> max;
                std::vector<float> colour = {0.3f,0.3f,0.0f};
                float intensity;

                if(data.substr(index,data.size()).find("min") != -1){
                    innerIndex = data.substr(index,data.size()).find("min")+5;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            min.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    min.push_back(std::stof(num));
                    num.clear();
                }

                if(data.substr(index,data.size()).find("max") != -1){
                    innerIndex = data.substr(index,data.size()).find("max")+5;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            max.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    max.push_back(std::stof(num));
                    num.clear();
                }

                if(data.substr(index,data.size()).find("color") != -1){
                    innerIndex = data.substr(index,data.size()).find("color")+7;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            colour.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    colour.push_back(std::stof(num));
                    num.clear();
                }

                cube = Cube(glm::vec3(colour[0], colour[1], colour[2]));
                glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(max[0]-min[0], max[1]-min[1], max[2]-min[2]));
                M = glm::translate(M,glm::vec3(min[0] + (max[0]-min[0])/2.0f ,min[1] + (max[1]-min[1])/2.0f,min[2] + (max[2]-min[2])/2.0f));
                cube.transform(M);

            }
            currStr.clear();
            index = innerIndex;
        }
        index++;
    }

    return cube;
}

Camera Scene::getCameraFromXML(std::string data) {
    int index = 0;
    int innerIndex = 0;
    std::string currStr;

    Camera cam;


    while(index != data.size()-1){

        if(data[index] == '<' && data[index+1] != '/' && data[index+1] != '?' && data[index+1] != '!'){

            innerIndex = index+1;
            while(data[innerIndex] != ' ' && data[innerIndex] != '\t'){
                currStr.push_back(data[innerIndex]);
                innerIndex++;
            }

            //std::cout<<"CURRENT STRING IS: "<<currStr<<std::endl;

            if(currStr == "camera"){
                innerIndex = 0;
                std::string name;
                std::string num;
                std::vector<float> from;
                std::vector<float> to;
                std::vector<float> up;
                float fovy = 45.0f;

                innerIndex = data.substr(index,data.size()).find("name")+6;
                while(data[innerIndex] != '\"' ){
                    name.push_back(data[innerIndex]);
                    innerIndex++;
                }

                if(data.substr(index,data.size()).find("from") != -1){
                    innerIndex = data.substr(index,data.size()).find("from")+6;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            from.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    from.push_back(std::stof(num));
                    num.clear();
                } else {
                    from = {0.0f,1.5f,4.0f};
                }

                if(data.substr(index,data.size()).find("to") != -1){
                    innerIndex = data.substr(index,data.size()).find("to")+4;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            to.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    to.push_back(std::stof(num));
                    num.clear();
                } else {
                    to = {0.0f,0.0f,0.0f};
                }

                if(data.substr(index,data.size()).find("up") != -1){
                    innerIndex = data.substr(index,data.size()).find("up")+4;
                    while(data[innerIndex] != '\"' ){
                        if(data[innerIndex] != ' '){
                            num.push_back(data[innerIndex]);
                        } else {
                            up.push_back(std::stof(num));
                            num.clear();
                        }
                        innerIndex++;
                    }
                    up.push_back(std::stof(num));
                    num.clear();
                } else {
                    up = {0.0f,1.0f,0.0f};
                }

                if(data.substr(index,data.size()).find("fovy") != -1){
                    innerIndex = data.substr(index,data.size()).find("fovy")+6;
                    while(data[innerIndex] != '\"' ){
                        num.push_back(data[innerIndex]);
                        innerIndex++;
                    }
                    fovy = std::stof(num);
                    num.clear();
                }

                cam = Camera(glm::vec3(from[0],from[1],from[2]),glm::vec3(to[0],to[1],to[2]), glm::vec3(up[0],up[1],up[2]),fovy, width, height);

            }
            currStr.clear();
            index = innerIndex;
        }
        index++;
    }

    return cam;
}

Node Scene::getNodeFromXML(std::string data) {
    std::vector<std::string> stack;
    std::string currStr;
    std::string name;
    std::vector<float> rotation;
    std::vector<float> translation;
    std::string ref;
    Node parent;

    for(int i = 0; i<data.size(); i++){
        if(data[i] == '<' && data[i+1] != '/' && data[i+1] != '?' && data[i+1] != '!'){
            int j=i;
            std::string entry;
            while(j != data.size()){
                //std::cout<<data[j]<<std::endl;
                j = i+1;
                while(data[j] != ' ' && data[j] != '\t'){
                    currStr.push_back(data[j]);
                    j++;
                }

                int innerIndex = data.substr(j,data.size()).find("name")+6;
                while(data[innerIndex] != '\"' ){
                    name.push_back(data[innerIndex]);
                    innerIndex++;
                }

                if(currStr == "node"){
                    int indx = data.substr(j,data.size()-j).find('>');
                    //TODO: add transform for the node
                    Node child = getNodeFromXML(data.substr(indx,data.size()-indx));
                    //child.name = ;
                }

                if(currStr == "plane"){
                    CheckerPlane p = CheckerPlane();
                    p.transform(glm::rotate(glm::mat4(1.0f),glm::radians(90.0f),glm::vec3(1.0f,0.0f,0.0f)));
                    parent.addChild(p);
                }

                if(data[j] == '>'){
                    entry.push_back(data[j]);
                    //list.push_back(entry);
                    break;
                } else {
                    entry.push_back(data[j]);
                    j++;
                }
            }
            i = j;
        }
    }
    return parent;
}

std::string Scene::getNodeStringParameter(std::string parameter, std::string data) {

    std::string result;
    int index = 0;

    if(data.find(parameter) != -1){
        int innerIndex = data.find(parameter) + parameter.size() + 2;
        while(data[innerIndex] != '\"' ){
            result.push_back(data[innerIndex]);
            innerIndex++;
        }
    }

    return result;
}

glm::vec3 Scene::getNodeVec3Paramter(std::string parameter, std::string data) {

    std::vector<float> tempVec;
    std::string num;

    if(data.find(parameter) != -1){
        int innerIndex = data.find(parameter) + parameter.size() + 2;
        while(data[innerIndex] != '\"' ){
            if(data[innerIndex] != ' '){
                num.push_back(data[innerIndex]);
            } else {
                tempVec.push_back(std::stof(num));
                num.clear();
            }
            innerIndex++;
        }
        tempVec.push_back(std::stof(num));
        num.clear();
    }

    return glm::vec3(tempVec[0],tempVec[1],tempVec[2]);
}

float Scene::getNodeFloatParamter(std::string parameter, std::string data) {
    int index = 0;
    std::string num;

    int innerIndex = data.find(parameter) + parameter.size() + 2;
    while(data[innerIndex] != '\"' ){
        num.push_back(data[innerIndex]);
        innerIndex++;
    }
    return std::stof(num);
}

