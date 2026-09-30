#pragma once
#include "GraphicsDef.h"

class Camera
{
public:
    Camera(vector3_t position = {0,0,0}, vector3_t worldUp = { 0.0f, 1.0f, 0.0f });

    //rotate camera (using radians)
    void Rotate(float xRot, float yRot);

    XMMATRIX GetView();
    inline vector3_t Position() { return _eye; }

    inline vector3_t Forward() { return _forward; }

private:
    vector3_t _eye;
    vector3_t _forward;
    vector3_t _worldUp;

};
