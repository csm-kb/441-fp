//
// Created by snaki on 12/12/2020.
//

#ifndef FP_KANIMHANDLER_H
#define FP_KANIMHANDLER_H

#include <functional>

namespace kVox {
    typedef std::function<void(double)> AnimHandler_t;

    /**
     * The default super for animation handlers. Time-based.
     */
    class kAnimHandler {
    public:
        kAnimHandler() = delete;
        explicit kAnimHandler( const AnimHandler_t& f ) : handler(f) {}
        bool state() { return isEnabled; }
        void enable() { isEnabled = true; }
        void disable() { isEnabled = false; }
        virtual void reset() { timeElapsed = 0.0; }
        virtual void update(double _tDelta) {
            timeElapsed += _tDelta;
            handler(_tDelta);
        }
    protected:
        bool isEnabled = false;
        const AnimHandler_t& handler;
    private:
        double timeElapsed;
    };

    /**
     * A state-based animation handler, where an interpolation in the range [0.0,1.0] determines animation state.
     */
    class kStateAnimHandler : public kAnimHandler {
    public:
        kStateAnimHandler() = delete;
        explicit kStateAnimHandler( const AnimHandler_t& f ) : kAnimHandler(f) {}
        void reset() override { interp = 0.0; }
        void update(double _interp) override {
            interp = _interp;
            handler(_interp);
        }
    private:
        double interp;
    };
}

#endif //FP_KANIMHANDLER_H
