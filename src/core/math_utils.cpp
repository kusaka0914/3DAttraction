#include "math_utils.h"

namespace core {
namespace math {

Mat4 createTransform(const Vec3& position, const Quat& rotation, const Vec3& scale) {
    Mat4 transform = glm::translate(Mat4(1.0f), position);
    transform = transform * glm::toMat4(rotation);
    transform = glm::scale(transform, scale);
    return transform;
}

Mat4 createLookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
    return glm::lookAt(eye, center, up);
}

Mat4 createPerspective(float fov, float aspect, float near, float far) {
    return glm::perspective(fov, aspect, near, far);
}

Mat4 createOrthographic(float left, float right, float bottom, float top, float near, float far) {
    return glm::ortho(left, right, bottom, top, near, far);
}

bool rayPlaneIntersection(const Ray& ray, const Plane& plane, float& t) {
    float denom = glm::dot(ray.direction, plane.normal);
    if (std::abs(denom) < 1e-6f) {
        return false; // レイと平面が平行
    }
    
    t = (plane.distance - glm::dot(ray.origin, plane.normal)) / denom;
    return t >= 0.0f;
}

bool raySphereIntersection(const Ray& ray, const Vec3& center, float radius, float& t) {
    Vec3 oc = ray.origin - center;
    float a = glm::dot(ray.direction, ray.direction);
    float b = 2.0f * glm::dot(oc, ray.direction);
    float c = glm::dot(oc, oc) - radius * radius;
    
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        return false;
    }
    
    float sqrtDisc = std::sqrt(discriminant);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    
    if (t1 > t2) std::swap(t1, t2);
    
    if (t1 < 0) {
        t1 = t2;
        if (t1 < 0) return false;
    }
    
    t = t1;
    return true;
}

Mat4 createPortalTransform(const Mat4& portalA, const Mat4& portalB) {
    // ポータルAの逆行列
    Mat4 invA = glm::inverse(portalA);
    
    // ポータルBの行列
    Mat4 B = portalB;
    
    // 面通過時の法線反転を含む回転行列
    Mat4 flipRotation = Mat4(1.0f);
    flipRotation[2][2] = -1.0f; // Z軸反転
    
    // 変換行列: M_B * R_flip * M_A^(-1)
    return B * flipRotation * invA;
}

} // namespace math
} // namespace core
