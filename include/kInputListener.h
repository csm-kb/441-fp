//
// Created by snaki on 12/6/2020.
//

#ifndef FP_KINPUTLISTENER_H
#define FP_KINPUTLISTENER_H

#include <SDL2/SDL.h>
#include <functional>
#include <memory>

namespace kVox {

    typedef std::function<void(const bool,const SDL_KeyboardEvent)> KeyInputCallback_t;
    typedef std::function<void(const SDL_MouseButtonEvent)> MouseButtonCallback_t;
    typedef std::function<void(const SDL_MouseMotionEvent)> MouseMotionCallback_t;

    class kKeyInputListener {
    public:
        kKeyInputListener() = delete;
        explicit kKeyInputListener( const KeyInputCallback_t& f ) : callback(f) {}
        void update(const bool isPressed, const SDL_KeyboardEvent key) {
            callback(isPressed, key);
        };
    private:
        const KeyInputCallback_t& callback;
    };

    class kMouseButtonListener {
    public:
        kMouseButtonListener() = delete;
        explicit kMouseButtonListener( const MouseButtonCallback_t& f ) : callback(f) {}
        void update(const SDL_MouseButtonEvent button) {
            callback(button);
        };
    private:
        const MouseButtonCallback_t& callback;
    };

    class kMouseMotionListener {
    public:
        kMouseMotionListener() = delete;
        explicit kMouseMotionListener( const MouseMotionCallback_t& f ) : callback(f) {};
        void update(const SDL_MouseMotionEvent event) {
            callback(event);
        }
    private:
        const MouseMotionCallback_t& callback;
    };

}

#endif //FP_KINPUTLISTENER_H
