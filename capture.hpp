#ifndef CAPTURE_HPP
#define CAPTURE_HPP
#include <X11/Xlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdamage.h>
#include <stdio.h>
#include "video-stream.hpp"
#include "nvfbchwenc.hpp"
class Capture{
    public:
        Capture();
        ~Capture();
        bool init();
        void do_capture();
        bool create_gpu_encoder();
        void destroy_gpu_encoder();
    private:
        Display * display;
        int screen_id;
        Window win;
        Damage damage;
        XEvent event;
        int error_base;
        int xfixes_event_base;
        int xdamage_event_base;
        Window root;
        Window child;
        int last_x;
        int last_y;
        VideoStream* video_stream;
        NvFBCHwEnc* nvenc;
};

#endif // CAPTURE_HPP
