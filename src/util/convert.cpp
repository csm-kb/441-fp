//
// Created by snaki on 12/7/2020.
//

#include <util/convert.h>

namespace kVox::util {
    glm::vec3 SpherToCart(double theta, double phi) {
        return glm::vec3(cos(phi) * sin(theta),
            sin(phi),
            cos(phi) * cos(theta));
    }
}