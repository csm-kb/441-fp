//
// Created by snaki on 12/7/2020.
//

#ifndef FP_CAMERA_H
#define FP_CAMERA_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <util/convert.h>

namespace kVox {

    /**
     * The base camera object, for use by the renderer in determining what the viewport shows.<br>
     * <br>
     * Note that there must be at least one default camera that the renderer can use!
     */
    class Camera {
    public:
        /** camera position in cartesian coords */
        glm::vec3 camPos;
        /** camera look-at position in cartesian coords */
        glm::vec3 camLookAt;
        /** camera direction in spherical coords */
        GLdouble cameraTheta, cameraPhi;
        /** camera distance in cartesian coords */
        GLdouble camDist;
        /** should this camera use mouse look? */
        bool canLook = false;
        /** should camera's orientation be locked w.r.t. point? */
        bool orientLocked = false;
        const glm::vec3* orientPos;

        /** Recomputes the camera position given its look-at target and/or orientation. */
        virtual void RecomputeCamPos() {
            // make sure we are looking at our target
            if (lookingAtTgt)
                camLookAt = (orientLocked) ? (*tgtLookAt + (*orientPos * glm::vec3(4.0))) : *tgtLookAt;
            else {
                camLookAt = camPos + util::SpherToCart(cameraTheta, cameraPhi);
            }
            // convert our theta and phi spherical angles to a cartesian vector, factoring in zoom,
            // and compute the camera's actual position
            if (!orientLocked)
                camPos = (glm::vec3(camDist) * glm::vec3(glm::sin(cameraTheta) * glm::sin(cameraPhi),
                                                     -glm::cos(cameraPhi),
                                                     -glm::cos(cameraTheta) * glm::sin(cameraPhi))) + camLookAt;
            else
                camPos = *tgtLookAt - (*orientPos * glm::vec3(glm::sqrt(camDist)));
        }

        /** Set's the camera's look-at target to be the reference target position. */
        void SetTargetLookAt(const glm::vec3* tgtPos) { tgtLookAt = tgtPos; }

        /** Gets whether or not the camera is looking at a target. */
        bool GetLookingAtTgt() { return lookingAtTgt; }
        /** Sets whether or not the camera is looking at a target. */
        void SetLookingAtTgt(bool looking) { lookingAtTgt = looking; }

    protected:
        bool lookingAtTgt = false;
        /** reference to camera target's position */
        const glm::vec3* tgtLookAt;
    };
}

#endif //FP_CAMERA_H
