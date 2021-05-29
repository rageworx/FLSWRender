#include "geometry.hpp"

#define R2PI    ( M_PI / 180.0 )

mat geometry::computeModelMatrix(const sceneparam& m)
{
    mat ScaleMatrix = mat::identity(4);
    ScaleMatrix[0][0] = m.meshScale.x;
    ScaleMatrix[1][1] = m.meshScale.y;
    ScaleMatrix[2][2] = m.meshScale.z;

    mat MoveMatrix = mat::identity(4);
    MoveMatrix[0][3] = m.meshMove.x;
    MoveMatrix[1][3] = m.meshMove.y;
    MoveMatrix[2][3] = m.meshMove.z;

    mat RotateMatrix = mat::identity(4);
    
    float cosX = std::cos(m.meshRotate.x * R2PI);
    float sinX = std::sin(m.meshRotate.x * R2PI);

    float cosY = std::cos(m.meshRotate.y * R2PI);
    float sinY = std::sin(m.meshRotate.y * R2PI);

    float cosZ = std::cos(m.meshRotate.z * R2PI);
    float sinZ = std::sin(m.meshRotate.z * R2PI);

    RotateMatrix[0][0] = cosY*cosZ;
    RotateMatrix[0][1] = -cosX*sinZ + sinX*sinY*cosZ;
    RotateMatrix[0][2] = sinX*sinZ + cosX*sinY*cosZ;

    RotateMatrix[1][0] = cosY*sinZ;
    RotateMatrix[1][1] = cosX*cosZ + sinX*sinY*sinZ;
    RotateMatrix[1][2] = -sinX*cosZ + cosX*sinY*sinZ;

    RotateMatrix[2][0] = -sinY;
    RotateMatrix[2][1] = sinX*cosY;
    RotateMatrix[2][2] = cosX*cosY;

    return MoveMatrix * RotateMatrix * ScaleMatrix;
}

mat geometry::computeViewMatrix(const sceneparam& m)
{
    vec3f n = (m.eye - m.at).normalize();     /// == Z
    vec3f u = (m.up ^ n).normalize();           /// == X
    vec3f v = (n ^ u).normalize();              /// == Y

    mat viewMat = mat::identity(4);
    viewMat[0][0] = u.x;
    viewMat[0][1] = u.y;
    viewMat[0][2] = u.z;
    viewMat[0][3] =  -(m.eye * u);

    viewMat[1][0] = v.x;
    viewMat[1][1] = v.y;
    viewMat[1][2] = v.z;
    viewMat[1][3] =  -(m.eye * v);

    viewMat[2][0] = n.x;
    viewMat[2][1] = n.y;
    viewMat[2][2] = n.z;
    viewMat[2][3] =  -(m.eye * n);

    return viewMat;
}

mat geometry::computeProjectMatrix(const sceneparam& m)
{
    mat projMat = mat::identity(4);

    // cot(fovy* pi / 360)
    float cotFovyDiv2 = 1.f / tan(m.fovY*R2PI / 2.f);

    projMat[0][0] = cotFovyDiv2 / m.aspect;
    projMat[1][1] = cotFovyDiv2;
    projMat[2][2] = m.farZ / (m.farZ - m.nearZ);
    projMat[2][3] = m.nearZ*m.farZ / (m.farZ - m.nearZ);
    projMat[3][2] = -1;
    projMat[3][3] = 0;

    return projMat;
}

mat geometry::computeViewportMatrix(float minX,float minY,float maxZ,float minZ,int w,int h)
{
    mat viewport = mat::identity(4);
    viewport[0][0] = w/2.f;
    viewport[1][1] = h/2.f;
    viewport[2][2] = maxZ - minZ;
    viewport[0][3] = minX + w/2;
    viewport[1][3] = minY + h/2;
    viewport[2][3] = minZ;
    return viewport;
}

mat geometry::computeMVP(const sceneparam& sence)
{
    mat M = computeModelMatrix(sence);
    mat V = computeViewMatrix(sence);
    mat P = computeProjectMatrix(sence);

    return P * V * M;
}

