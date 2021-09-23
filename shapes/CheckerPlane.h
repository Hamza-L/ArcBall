//
// Created by Hamza Lahmimsi on 2021-08-26.
//

#ifndef ARCBALL_CHECKERPLANE_H
#define ARCBALL_CHECKERPLANE_H

#include "Plane.h"

class CheckerPlane : public Node {
public:
    CheckerPlane(){
        int size = 10;

        glm::vec4 colour1 = {0.2f,0.2f,0.2f,1.0f};
        glm::vec4 colour2 = {0.1f,0.1f,0.1f,1.0f};

        for(int i = -size; i<size; i++){
            for(int j = -size; j<size; j++){
                glm::vec4 currColour;
                if((i+j)%2 == 0){
                    currColour = colour1;
                } else {
                    currColour = colour2;
                }
                Plane p = Plane(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f+i,0.5f+j,0.0f)), currColour);
                addChild(p);
            }
        }
    }

    CheckerPlane(glm::vec4 colour1, glm::vec4 colour2, int size){

        for(int i = -size; i<size; i++){
            for(int j = -size; j<size; j++){
                glm::vec4 currColour;
                if((i-j)%2 == 0){
                    currColour = colour1;
                } else {
                    currColour = colour2;
                }
                Plane p = Plane(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f+i,0.5f+j,0.0f)), currColour);
                addChild(p);
            }
        }
    }

};


#endif //ARCBALL_CHECKERPLANE_H
