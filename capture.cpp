//#include <config.h>
//#include <glib.h>
#include "stdlib.h"
#include "string.h"
#include "capture.hpp"
#include <time.h>

typedef unsigned char  BYTE;
typedef unsigned short	WORD;
typedef unsigned int  DWORD;
extern FILE* f_log;

#define PACKED __attribute__(( packed, aligned(2)))

typedef struct tagBITMAPFILEHEADER{
     WORD     bfType;        //Linux此值为固定值，0x4d42
     DWORD    bfSize;        //BMP文件的大小，包含三部分
     WORD     bfReserved1;    //置0
     WORD     bfReserved2;
     DWORD    bfOffBits;     //文件起始位置到图像像素数据的字节偏移量

}PACKED BITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER{
     DWORD    biSize;          //文件信息头的大小，40
     DWORD    biWidth;         //图像宽度
     DWORD    biHeight;        //图像高度
     WORD     biPlanes;        //BMP存储RGB数据，总为1
     WORD     biBitCount;      //图像像素位数，笔者RGB位数使用24
     DWORD    biCompression;   //压缩 0：不压缩  1：RLE8 2：RLE4
     DWORD    biSizeImage;     //4字节对齐的图像数据大小
     DWORD    biXPelsPerMeter; //水平分辨率  像素/米
     DWORD    biYPelsPerMeter;  //垂直分辨率  像素/米
     DWORD    biClrUsed;        //实际使用的调色板索引数，0：使用所有的调色板索引
     DWORD    biClrImportant;
}BITMAPINFOHEADER;

static void dump_bmp(char* buffer, int width, int height)
{
    static int id = 0;
    char file[200];
    sprintf(file, "/tmp/tmpfs/%d.bmp", id++);

    FILE* f = fopen(file, "wb");
    if (!f) {
       return;
    }

    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = 0;
    bi.biSizeImage = width*height*4;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    BITMAPFILEHEADER bf;
    bf.bfType = 0x4d42;
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + bi.biSizeImage;
    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;

    fwrite(&bf,sizeof(BITMAPFILEHEADER),1,f);                      //写入文件头
    fwrite(&bi,sizeof(BITMAPINFOHEADER),1,f);                      //写入信息头
    fwrite(buffer,bi.biSizeImage,1,f);

    fclose(f);
}

static inline int get_time(void)
{
    timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);

    return (int)now.tv_sec * 1000 + (int)now.tv_nsec / 1000000;

}

Capture::Capture()
{

}
bool Capture::init()
{
    display = XOpenDisplay(":0.0");
    if (!display){
        return false;
    }
    screen_id = DefaultScreen(display);
    win = RootWindow(display, screen_id);
    XWindowAttributes win_info;
    XGetWindowAttributes(display, win, &win_info);
    XSelectInput(display, win, StructureNotifyMask);
    if (!XFixesQueryExtension(display, &xfixes_event_base, &error_base)) {
        return false;
    }
    XFixesSelectCursorInput(display, DefaultRootWindow(display), XFixesDisplayCursorNotifyMask);
    if (!XDamageQueryExtension(display, &xdamage_event_base, &error_base)) {
        return false;
    }
    damage = XDamageCreate(display, win, XDamageReportRawRectangles);
    last_x = -1;
    last_y = -1;
    video_stream = new VideoStream();
    nvenc = NULL;
}

Capture::~Capture()
{
   
    if (damage){
        XDamageDestroy(display, damage);
    }
    if (video_stream){
        delete video_stream;
    }
    if (display){
        XCloseDisplay(display);
    } 
    if (nvenc){
        destroy_gpu_encoder();
    }
}

void Capture::do_capture()
{
   static int count_cursor = 2000;
   int time1 = get_time();
   static bool stream_start = false;
   if (video_stream){
        video_stream->Stream_Timeout(time1);
   }

    int r_x, r_y, c_x, c_y;unsigned int mask;
    XQueryPointer(display, win, &root, &child, &r_x, &r_y, &c_x, &c_y, &mask);
    if (last_x != r_x || last_y != r_y){
        last_x = r_x;
        last_y = r_y;
    }

   XNextEvent(display, &event);
    if (event.type == xfixes_event_base + 1) {
        XFixesCursorImage *cursor = XFixesGetCursorImage(display);
        if (cursor){
        // cursor->x, cursor->y, cursor->xhot, cursor->yhot, cursor->width, cursor->height, cursor->pixels);
        //光标改变写串口发一次分辨率信息


        }
    } else if (event.type == ConfigureNotify){
        XConfigureEvent *config = &capture->event.xconfigure;
        if (config)
        {
               printf("capture win width: %d height: %d\n", config->width, config->height);
             //分辨率发送变化写串口发一次分辨率信息
               if (video_stream){
                   video_stream->Stream_Reset();
               }

        }
    } else if (event.type == capture->xdamage_event_base + XDamageNotify){
        XDamageNotifyEvent *damage_event = (XDamageNotifyEvent*) (&event);
        Rect rect;
        rect.top =  damage_event->area.y;
        rect.bottom =damage_event->area.y+damage_event->area.height;
        rect.left = damage_event->area.x;
        rect.right = damage_event->area.y+damage_event->area.width;
        int time2 = get_time();
        if (video_stream){
            video_stream->Add_Stream(&rect, time2);
        }
        stream_start = video_stream->Is_StreamStart();
        if (stream_start){
                printf("stream start!\n");
        } else {
                printf( "stream stop!\n");
        }
        if (!stream_start){
            destroy_gpu_encoder();
            XImage *image = XGetImage(display, damage_event->drawable, damage_event->area.x, damage_event->area.y,
                      damage_event->area.width, damage_event->area.height, AllPlanes, ZPixmap);
            if (!image){
                printf("capture x get image failed!");
                return;
            }
            printf("capture image\n");
        } else {
            if (nvenc == NULL){
                if (!create_gpu_encoder(){
                    printf("create encoder failed!");
                    destroy_gpu_encoder();
                }
            }
            if (nvenc){
                nvenc->gpu_capture();
            }
        }

    } else {

    }
}

bool Capture::create_gpu_encoder()
{
    nvenc = new NvFBCHwEnc();
    if (nvenc->init()){
        return true;
    }
    destroy_gpu_encoder();
    return false;
}
void Capture::destroy_gpu_encoder()
{   
    if (nvenc){
        delete nvenc;
        nvenc = NULL;
    }
}