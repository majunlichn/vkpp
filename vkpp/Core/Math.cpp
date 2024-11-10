#include <vkpp/Core/Math.h>

namespace vkpp
{

glm::vec3 SphericalToCartesian(SphericalCoord sph)
{
    glm::vec3 cart;
    cart.x = sph.r * std::sin(sph.theta) * std::cos(sph.phi);
    cart.y = sph.r * std::sin(sph.theta) * std::sin(sph.phi);
    cart.z = sph.r * std::cos(sph.theta);
    return cart;
}

SphericalCoord CartesianToSpherical(glm::vec3 cart)
{
    float x = cart.x;
    float y = cart.y;
    float z = cart.z;
    SphericalCoord sph;
    sph.r = std::sqrt(x * x + y * y + z * z);
    sph.theta = std::acos(z / sph.r);
    sph.phi = std::atan2(y, x);
    return sph;
}

} // namespace vkpp
