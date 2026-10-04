#include "Camera.h"
#include <algorithm>
#include <iostream>

Camera::Camera(vector3_t position, vector3_t worldUp) {

    //Rotate(yaw, pitch);

    _forward = { 0,0,1 };

    _eye = position;
    _worldUp = worldUp;

    RecalculateRightVec();

    //DirectX::XMStoreFloat4x4(&frameData.view, DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(_eye, _eye + _forward, _worldUp)));
}


//rotate camera (using radians)
void Camera::Rotate(float yaw, float pitch) {

    pitch += asin(_forward.y);
    yaw += atan2(_forward.z, _forward.x);
    //std::cout << yaw << std::endl;

    pitch = std::clamp<float>(pitch, -(DirectX::XM_PIDIV2 - 0.1f), DirectX::XM_PIDIV2 - 0.1f);

    _forward.x = cosf(pitch) * cosf(yaw);
    _forward.y = sinf(pitch);
    _forward.z = cosf(pitch) * sinf(yaw);

    RecalculateRightVec();
}


XMMATRIX Camera::GetView() {

    DirectX::XMVECTOR eye = DirectX::XMLoadFloat3(&_eye);
    DirectX::XMVECTOR at = eye + DirectX::XMLoadFloat3(&_forward);
    DirectX::XMVECTOR worldUp = DirectX::XMLoadFloat3(&_worldUp);

    return DirectX::XMMatrixTranspose(DirectX::XMMatrixLookAtRH(eye, at, worldUp));
    //return ::XMMatrixLookAtRH(eye, at, worldUp);
}