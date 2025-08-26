#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace core {

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;
using Quat = glm::quat;

// 定数
constexpr float PI = 3.14159265359f;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;

// 数学ユーティリティ関数
namespace math {

// 角度変換
inline float toRadians(float degrees) { return degrees * DEG_TO_RAD; }
inline float toDegrees(float radians) { return radians * RAD_TO_DEG; }

// 行列作成
Mat4 createTransform(const Vec3& position, const Quat& rotation = Quat(), const Vec3& scale = Vec3(1.0f));
Mat4 createLookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
Mat4 createPerspective(float fov, float aspect, float near, float far);
Mat4 createOrthographic(float left, float right, float bottom, float top, float near, float far);

// 平面
struct Plane {
    Vec3 normal;
    float distance;
    
    Plane() : normal(0, 1, 0), distance(0) {}
    Plane(const Vec3& n, float d) : normal(glm::normalize(n)), distance(d) {}
    Plane(const Vec3& point, const Vec3& n) : normal(glm::normalize(n)), distance(glm::dot(normal, point)) {}
    
    float distanceToPoint(const Vec3& point) const {
        return glm::dot(normal, point) - distance;
    }
};

// レイ
struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(glm::normalize(d)) {}
    
    Vec3 getPoint(float t) const { return origin + direction * t; }
};

// 交差判定
bool rayPlaneIntersection(const Ray& ray, const Plane& plane, float& t);
bool raySphereIntersection(const Ray& ray, const Vec3& center, float radius, float& t);

// ポータル変換行列
Mat4 createPortalTransform(const Mat4& portalA, const Mat4& portalB);

} // namespace math

} // namespace core
