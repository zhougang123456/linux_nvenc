#include "video-stream.hpp"
#include "stdlib.h"
#include "string.h"

         bool VideoStream::Rect_Is_Equal(Rect* rect1, Rect* rect2)
        {
            if (rect1->left == rect2->left && rect1->right == rect2->right
                && rect1->top == rect2->top && rect1->bottom == rect2->bottom)
            {
                return true;
            }
            return false;
        }

        bool VideoStream::Rect_Is_Contain(Rect* rect1, Rect* rect2)
        {
            if (rect1->left <= rect2->left && rect1->right >= rect2->right
                && rect1->top <= rect2->top && rect1->bottom >= rect2->bottom)
            {
                return true;
            }
            return false;
        }

        void VideoStream::Copy_Rect(Rect* dest, Rect* src)
        {
            dest->left = src->left;
            dest->right = src->right;
            dest->top = src->top;
            dest->bottom = src->bottom;
        }

        VideoStream::VideoStream()
        {
            m_candidate.clear();
            m_stream.clear();
            memset(m_stream_id, 0, sizeof(m_stream_id));
        }

        VideoStream::~VideoStream()
        {
            Candidates::iterator candidate_iter;
            for (candidate_iter = m_candidate.begin(); candidate_iter != m_candidate.end(); )
            {
                Candidate* candidate = (Candidate*)*candidate_iter;
                candidate_iter = m_candidate.erase(candidate_iter);
                free(candidate);
            }
            Streams::iterator stream_iter;
            for (stream_iter = m_stream.begin(); stream_iter != m_stream.end(); )
            {
                Stream* stream = (Stream*)*stream_iter;
                stream_iter = m_stream.erase(stream_iter);
                free(stream);
            }

        }

        void VideoStream::Add_Stream(Rect* rect, int time)
        {

            //STAT_LOG("add stream time %d", time);
            bool Is_Candidate = false;
            if ((rect->right - rect->left) * (rect->bottom - rect->top) > STREAM_MIN_SIZE)
            {
                Is_Candidate = true;
            }
            Streams::iterator stream_iter;
            for (stream_iter = m_stream.begin();
                stream_iter != m_stream.end(); )
            {
                Stream* stream = (Stream*)*stream_iter;
                if (time - stream->time > MAX_STREAM_DETECT_TIME)
                {
                    Set_StreamId(stream->id, false);
                    stream_iter = m_stream.erase(stream_iter);
                    //STAT_LOG("remove stream %d!", stream->id);
                    free(stream);
                }
                else if (Is_Candidate && Rect_Is_Contain(rect, &stream->rect))
                {
                    stream->time = time;
                    stream_iter++;
                }
                else
                {
                    stream_iter++;
                }
            }
            if (m_candidate.empty())
            {
                //STAT_LOG("create a new candidate!");
                Candidate* candidate = (Candidate*)malloc(sizeof(Candidate));
                Copy_Rect(&candidate->rect, rect);
                candidate->time = time;
                candidate->count = 1;
                m_candidate.push_back(candidate);
                //m_candidate.remove(candidate);
            }
            else
            {
                Candidates::iterator candidate_iter;
                bool Is_Stream = false;
                for (candidate_iter = m_candidate.begin();
                    candidate_iter != m_candidate.end(); )
                {
                    Candidate* candidate = reinterpret_cast<Candidate*>(*candidate_iter);
                    if (time - candidate->time > MAX_DETECT_TIME)
                    {
                        //printf("m_candidate size %d \n", (int)m_candidate.size());
                        candidate_iter = m_candidate.erase(candidate_iter);
                        free(candidate);
                    }
                    else if (Is_Candidate && Rect_Is_Equal(rect, &candidate->rect))
                    {
                        Is_Stream = true;
                        candidate->count++;
                        candidate->time = time;
                        if (candidate->count >= STREAM_START_CONDITION)
                        {
                            if (Get_StreamId() >= 0) {
                                Stream* stream = (Stream*)malloc(sizeof(Stream));
                                Copy_Rect(&stream->rect, rect);
                                stream->time = time;
                                stream->id = Get_StreamId();
                                Set_StreamId(stream->id, true);
                                m_stream.push_back(stream);
                                //STAT_LOG("create a new stream!");
                            } else {
                                //STAT_LOG("get stream id failed!");
                            }
                            candidate_iter = m_candidate.erase(candidate_iter);
                            free(candidate);
                        }
                        else {
                            candidate_iter++;
                        }
                    }
                    else {
                        candidate_iter++;
                    }
                    //printf("m_candidate size %d \n", (int)m_candidate.size());
                }
                //STAT_LOG("m_candidate size %d!", m_candidate.size());
                if (!Is_Stream)
                {
                    //STAT_LOG("create a new candidate!");
                    Candidate* candidate = (Candidate*)malloc(sizeof(Candidate));
                    Copy_Rect(&candidate->rect, rect);
                    candidate->time = time;
                    candidate->count = 1;
                    m_candidate.push_back(candidate);
                }
            }

        }

        bool VideoStream::Is_StreamStart(void)
        {
            //STAT_LOG("stream size  %d", m_stream.size());
            //printf("m_stream size %d \n", (int)m_stream.size());
            if (m_stream.empty()) {
                return false;
            }
            return true;
        }

        void VideoStream::Stream_Timeout(int time)
        {
            Streams::iterator stream_iter;
            for (stream_iter = m_stream.begin();
                stream_iter != m_stream.end(); )
            {
                Stream* stream = (Stream*)*stream_iter;
                if (time - stream->time > MAX_STREAM_DETECT_TIME)
                {
                    //STAT_LOG("stream %d timeout", stream->id);
                    Set_StreamId(stream->id, false);
                    stream_iter = m_stream.erase(stream_iter);
                    free(stream);
                }
                else
                {
                    stream_iter++;
                }
            }
        }

        void VideoStream::Stream_Reset(void)
        {
            Candidates::iterator candidate_iter;
            for (candidate_iter = m_candidate.begin(); candidate_iter != m_candidate.end(); )
            {
                Candidate* candidate = (Candidate*)*candidate_iter;
                candidate_iter = m_candidate.erase(candidate_iter);
                free(candidate);
            }

            Streams::iterator stream_iter;
            for (stream_iter = m_stream.begin(); stream_iter != m_stream.end(); )
            {
                Stream* stream = (Stream*)*stream_iter;
                Set_StreamId(stream->id, false);
                stream_iter = m_stream.erase(stream_iter);
                free(stream);
            }
        }

        int VideoStream::Get_StreamId()
        {
            for (int i = 0; i < MAX_STREAMS; i++) {
                if (!m_stream_id[i]) {
                    return i;
                }
            }
            //STAT_LOG("stream is not enough!");
            return -1;
        }

        void VideoStream::Set_StreamId(int id, bool allowed)
        {
            m_stream_id[id] = allowed;

}
