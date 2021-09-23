#ifndef VULKANTESTING_OBJIMPORTER_H
#define VULKANTESTING_OBJIMPORTER_H

#include <iostream>
#include <fstream>
#include <string>

#include <utility>
#include <vector>
#include "Node.h"


class ObjImporter : public Node{
    public:
        //hva::VulkanModel object;
        std::string texName{};
        std::string UVTexName{};

        ObjImporter(std::string fileName, glm::vec4 colour){
            std::ifstream myfile("objects/"+ fileName + ".obj");
            std::string data;

            std::vector<glm::vec3> normals;
            std::vector<glm::vec3> positions;
            std::vector<glm::vec2> texture;

            std::vector<int> normInd;
            
            getline(myfile, data);

            int numslash,numspace;

            int counter = 0;

            while(!data.empty()){
                Vertex v0;
                if(data[0] == 'v' && data[1] == ' '){
                    std::string posNum;
                    std::vector<float> posList;
                    for(int i=2; i<data.size(); i++){
                        posNum.push_back(data[i]);
                        if(data[i] == ' ' || i==data.size()-1){
                            posNum.pop_back();
                            posList.push_back(std::stof(posNum));
                            posNum.clear();
                        }
                    }
                    positions.push_back(glm::vec3(posList[0],posList[1]+1.0f,posList[2]));
                }

                if(data[0] == 'v' && data[1] == 'n'){
                    std::string normNum;
                    std::vector<float> normList;
                    for(int i=3; i<data.size(); i++){
                        normNum.push_back(data[i]);
                        if(data[i] == ' ' || i==data.size()-1){
                            normNum.pop_back();
                            normList.push_back(std::stof(normNum));
                            normNum.clear();
                        }
                    }
                    normals.push_back(glm::vec3(normList[0],normList[1],normList[2]));
                }

                if(data[0] == 'f'){
                    int posInd1,posInd2,posInd3;
                    int normInd1,normInd2,normInd3;
                    std::string num;
                    numslash = 0;
                    numspace = 0;
                    for(int i = 2; i<data.size(); i++){

                        num.push_back(data[i]);

                        //std::cout<<"data[i]: "<<data[i]<<std::endl;

                        if(data[i] == '/'){
                            numslash++;
                            num.pop_back();
                            if(numslash == 1){
                                //std::cout<<num<<std::endl;
                                posInd1 = std::stoi(num);
                            } else if( numslash == 3){
                                //std::cout<<num<<std::endl;
                                posInd2 = std::stoi(num);
                            } else if( numslash == 5){
                                //std::cout<<num<<std::endl;
                                posInd3 = std::stoi(num);
                            }
                            
                            num.clear();
                        }
                        
                        if(data[i] == ' '){
                            numspace++;
                            num.pop_back();
                            if(numspace == 1){
                                //std::cout<<num<<std::endl;
                                normInd1 = std::stoi(num);
                            } else if( numspace == 2){
                                //std::cout<<num<<std::endl;
                                normInd2 = std::stoi(num);
                            } 
                            num.clear();

                        }
                        
                        if (i == (data.size()-1)){
                            //std::cout<<num<<std::endl;
                            normInd3 = std::stoi(num);
                            num.clear();
                        }
                        
                    }
                    num.clear();

                    Vertex v0,v1,v2;

                    indices.push_back(counter);
                    indices.push_back(counter+1);
                    indices.push_back(counter+2);

                    counter += 3;

                    v0.position = positions[posInd1-1];
                    v1.position = positions[posInd2-1];
                    v2.position = positions[posInd3-1];

                    v0.norm = normals[normInd1-1];
                    v1.norm = normals[normInd2-1];
                    v2.norm = normals[normInd3-1];

                    v0.colour = colour;
                    v1.colour = colour;
                    v2.colour = colour;

                    vertices.push_back(v0);
                    vertices.push_back(v1);
                    vertices.push_back(v2);

                    //std::cout<<"position: ["<<posInd1<<","<<posInd2<<","<<posInd3<<"]"<<std::endl;
                    //std::cout<<"normal: ["<<normInd1<<","<<normInd2<<","<<normInd3<<"]"<<std::endl;

                    //std::cout<<"position: ["<<vertices[posInd1].position.x<<","<<vertices[posInd1].position.y<<","<<vertices[posInd1].position.z<<"]"<<std::endl;
                    //std::cout<<"normal: ["<<vertices[posInd1].norm.x<<","<<vertices[posInd1].norm.y<<","<<vertices[posInd1].norm.z<<"]"<<std::endl;

                    //break;
                    //std::cout<<"position: ["<<posInd1<<","<<posInd2<<","<<posInd3<<"]"<<std::endl;
                    //std::cout<<"normal: ["<<normInd1<<","<<normInd2<<","<<normInd3<<"]"<<std::endl;
                }
                
                getline(myfile, data);
            }

            //std::cout<<"["<<pos1<<","<<pos2<<","<<pos3<<"]"<<std::endl;
            
            //pos1 = std::stof(data.substr(2,8));
            //pos2 = std::stof(data.substr(11,8));
            //pos3 = std::stof(data.substr(20,8));
            //std::cout<<"["<<pos1<<","<<pos2<<","<<pos3<<"]"<<std::endl;
            //v.norm;

            //vertices.push_back(v);
            //std::cout << data << std::endl;
            for( int i = 0; i<positions.size(); i++){
                //std::cout<<"position form list: ["<<positions[i].x<<","<<positions[i].y<<","<<positions[i].z<<"]"<<std::endl;
            }

            for( int i = 0; i<normals.size(); i++){
                //std::cout<<"normals form list: ["<<normals[i].x<<","<<normals[i].y<<","<<normals[i].z<<"]"<<std::endl;
            }

            for(int i = 0; i<5; i++){
                //std::cout<<"Index: "<<indices[i]<<std::endl;
                //std::cout<<"normalsList[1]: "<<normals[1].x<<","<<normals[1].y<<","<<normals[1].z<<"]"<<std::endl;
                //std::cout<<"position: ["<<vertices[indices[i]].position.x<<","<<vertices[indices[i]].position.y<<","<<vertices[indices[i]].position.z<<"]"<<std::endl;
                //std::cout<<"normal: ["<<vertices[indices[i]].norm.x<<","<<vertices[indices[i]].norm.y<<","<<vertices[indices[i]].norm.z<<"]"<<std::endl;
            }
        
            myfile.close();
        }

        ObjImporter(std::string fileName, std::string textureFile, std::string UVTextureFile){
        std::ifstream myfile("objects/"+ fileName + ".obj");
        std::string data;

        texName = std::move(textureFile);
        UVTexName = std::move(UVTextureFile);
        glm::vec4 colour = glm::vec4(0.1f,0.1f,0.1f,1.0f);

        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texture;

        std::vector<int> normInd;

        getline(myfile, data);

        int numslash,numspace;

        int counter = 0;

        while(!data.empty()){
            Vertex v0;
            if(data[0] == 'v' && data[1] == ' '){
                std::string posNum;
                std::vector<float> posList;
                for(int i=2; i<data.size(); i++){
                    posNum.push_back(data[i]);
                    if(data[i] == ' ' || i==data.size()-1){
                        posNum.pop_back();
                        posList.push_back(std::stof(posNum));
                        posNum.clear();
                    }
                }
                positions.push_back(glm::vec3(posList[0],posList[1]+1.0f,posList[2]));
            }

            if(data[0] == 'v' && data[1] == 'n'){
                std::string normNum;
                std::vector<float> normList;
                for(int i=3; i<data.size(); i++){
                    normNum.push_back(data[i]);
                    if(data[i] == ' ' || i==data.size()-1){
                        normNum.pop_back();
                        normList.push_back(std::stof(normNum));
                        normNum.clear();
                    }
                }
                normals.push_back(glm::vec3(normList[0],normList[1],normList[2]));
            }

            if(data[0] == 'f'){
                int posInd1,posInd2,posInd3;
                int normInd1,normInd2,normInd3;
                std::string num;
                numslash = 0;
                numspace = 0;
                for(int i = 2; i<data.size(); i++){

                    num.push_back(data[i]);

                    //std::cout<<"data[i]: "<<data[i]<<std::endl;

                    if(data[i] == '/'){
                        numslash++;
                        num.pop_back();
                        if(numslash == 1){
                            //std::cout<<num<<std::endl;
                            posInd1 = std::stoi(num);
                        } else if( numslash == 3){
                            //std::cout<<num<<std::endl;
                            posInd2 = std::stoi(num);
                        } else if( numslash == 5){
                            //std::cout<<num<<std::endl;
                            posInd3 = std::stoi(num);
                        }

                        num.clear();
                    }

                    if(data[i] == ' '){
                        numspace++;
                        num.pop_back();
                        if(numspace == 1){
                            //std::cout<<num<<std::endl;
                            normInd1 = std::stoi(num);
                        } else if( numspace == 2){
                            //std::cout<<num<<std::endl;
                            normInd2 = std::stoi(num);
                        }
                        num.clear();

                    }

                    if (i == (data.size()-1)){
                        //std::cout<<num<<std::endl;
                        normInd3 = std::stoi(num);
                        num.clear();
                    }

                }
                num.clear();

                Vertex v0,v1,v2;

                indices.push_back(counter);
                indices.push_back(counter+1);
                indices.push_back(counter+2);

                counter += 3;

                v0.position = positions[posInd1-1];
                v1.position = positions[posInd2-1];
                v2.position = positions[posInd3-1];

                v0.norm = normals[normInd1-1];
                v1.norm = normals[normInd2-1];
                v2.norm = normals[normInd3-1];

                v0.colour = colour;
                v1.colour = colour;
                v2.colour = colour;

                vertices.push_back(v0);
                vertices.push_back(v1);
                vertices.push_back(v2);

                //std::cout<<"position: ["<<posInd1<<","<<posInd2<<","<<posInd3<<"]"<<std::endl;
                //std::cout<<"normal: ["<<normInd1<<","<<normInd2<<","<<normInd3<<"]"<<std::endl;

                //std::cout<<"position: ["<<vertices[posInd1].position.x<<","<<vertices[posInd1].position.y<<","<<vertices[posInd1].position.z<<"]"<<std::endl;
                //std::cout<<"normal: ["<<vertices[posInd1].norm.x<<","<<vertices[posInd1].norm.y<<","<<vertices[posInd1].norm.z<<"]"<<std::endl;

                //break;
                //std::cout<<"position: ["<<posInd1<<","<<posInd2<<","<<posInd3<<"]"<<std::endl;
                //std::cout<<"normal: ["<<normInd1<<","<<normInd2<<","<<normInd3<<"]"<<std::endl;
            }

            getline(myfile, data);
        }

        //std::cout<<"["<<pos1<<","<<pos2<<","<<pos3<<"]"<<std::endl;

        //pos1 = std::stof(data.substr(2,8));
        //pos2 = std::stof(data.substr(11,8));
        //pos3 = std::stof(data.substr(20,8));
        //std::cout<<"["<<pos1<<","<<pos2<<","<<pos3<<"]"<<std::endl;
        //v.norm;

        //vertices.push_back(v);
        //std::cout << data << std::endl;
        for( int i = 0; i<positions.size(); i++){
            //std::cout<<"position form list: ["<<positions[i].x<<","<<positions[i].y<<","<<positions[i].z<<"]"<<std::endl;
        }

        for( int i = 0; i<normals.size(); i++){
            //std::cout<<"normals form list: ["<<normals[i].x<<","<<normals[i].y<<","<<normals[i].z<<"]"<<std::endl;
        }

        for(int i = 0; i<5; i++){
            //std::cout<<"Index: "<<indices[i]<<std::endl;
            //std::cout<<"normalsList[1]: "<<normals[1].x<<","<<normals[1].y<<","<<normals[1].z<<"]"<<std::endl;
            //std::cout<<"position: ["<<vertices[indices[i]].position.x<<","<<vertices[indices[i]].position.y<<","<<vertices[indices[i]].position.z<<"]"<<std::endl;
            //std::cout<<"normal: ["<<vertices[indices[i]].norm.x<<","<<vertices[indices[i]].norm.y<<","<<vertices[indices[i]].norm.z<<"]"<<std::endl;
        }

        myfile.close();
    }

        std::vector<Vertex> getFace(int index){
            std::vector<Vertex> face;

            face.push_back(vertices[index*3]);
            face.push_back(vertices[index*3+1]);
            face.push_back(vertices[index*3+2]);

            return face;
        }

        std::vector<uint32_t> getFaceIndx(int index){
            std::vector<uint32_t> face;

            face.push_back(index*3);
            face.push_back(index*3+1);
            face.push_back(index*3+2);

            return face;
        }
};

#endif //VULKANTESTING_OBJIMPORTER_H