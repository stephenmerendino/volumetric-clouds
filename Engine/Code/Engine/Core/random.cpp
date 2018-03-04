#include "Engine/Core/random.h"

Vector2 random2(const Vector2& p)
{
    float x = DotProduct(p, Vector2(127.1f, 311.7f));
    float y = DotProduct(p, Vector2(269.5f, 183.3f));
    Vector2 sin_vec = Vector2(sinf(x), sinf(y));
    sin_vec *= 43758.5453f;
    Vector2 ret;
    ret.x = sin_vec.x - (long)sin_vec.x;
    ret.y = sin_vec.y - (long)sin_vec.y;
    return ret;
}

Vector3 random3(const Vector3& p)
{
    float x = DotProduct(p, Vector3(127.1f, 311.7f, 204.3f));
    float y = DotProduct(p, Vector3(269.5f, 183.3f, 317.9f));
    float z = DotProduct(p, Vector3(334.7f, 113.4f, 253.1f));
    Vector3 sin_vec = Vector3(sinf(x), sinf(y), sinf(z));
    sin_vec *= 43758.5453f;
    Vector3 ret;
    ret.x = sin_vec.x - (long)sin_vec.x;
    ret.y = sin_vec.y - (long)sin_vec.y;
    ret.z = sin_vec.z - (long)sin_vec.z;
    return ret;
}