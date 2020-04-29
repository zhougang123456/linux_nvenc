#ifndef VIDEOSTREAM_H
#define VIDEOSTREAM_H
#include <list>
#define STREAM_START_CONDITION 30
#define MAX_DETECT_TIME 200
#define MAX_STREAM_DETECT_TIME 1000
#define MAX_STREAMS 50
#define STREAM_MIN_SIZE (200 * 200)


        typedef struct Rect
        {
            int left;
            int top;
            int right;
            int bottom;
        } Rect;
        typedef struct Candidate
        {
            Rect rect;
            int time;
            int count;
        }Candidate;
        typedef struct Stream
        {
            Rect rect;
            int id;
            int time;
        }Stream;
        typedef std::list<Candidate*> Candidates;
        typedef std::list<Stream*> Streams;

        class VideoStream
        {
        public:
            VideoStream();
            ~VideoStream();
            void Add_Stream(Rect* rect, int time); //dxgi接口获取到脏区域时调用，传递脏区域和当前时间
            bool Is_StreamStart(void);				 //图流模式判断，返回true为流模式，返回false为图模式
            void Stream_Timeout(int time);		 //每次dxgi循环开始时调用
            void Stream_Reset(void);				 //分辨率发生变化时调用

        private:
            bool Rect_Is_Equal(Rect* rect1, Rect* rect2);
            bool Rect_Is_Contain(Rect* rect1, Rect* rect2);
            void Copy_Rect(Rect* dest, Rect* src);

            Candidates m_candidate;
            Streams m_stream;
            bool m_stream_id[MAX_STREAMS];
            int Get_StreamId();
            void Set_StreamId(int id, bool allowed);
        };

#endif // VIDEOSTREAM_H

