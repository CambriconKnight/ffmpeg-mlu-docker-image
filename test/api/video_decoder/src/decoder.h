#ifndef _DECODE_H
#define _DECODE_H
#include "unistd.h"
#include "stdint.h"
#include "stdio.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

class Decoder {

public:
    int decode_process(std::string, int, int, bool);
    void print_perf_total();
    void print_perf_step();
    double get_actual_fps();
private:
    void close_decode();
    int init_decode(std::string);
    int find_video_stream_index();
    int decoding();
    void save_yuv_file(AVFrame *p_frame);

private:

    int video_stream_ = -1;

    AVFrame *p_frame_              = nullptr;
    AVFormatContext *p_format_ctx_ = nullptr;
    AVCodecContext  *p_codec_ctx_  = nullptr;

	int dev_id_           = 0;
    int thread_id_        = 0;
    int img_width_        = 0;
    int img_height_       = 0;
    int dec_num_          = 0;
    int step_dec_num_     = 0;
    bool is_dump_file_    = 0;
    double actual_fps_    = 0.0;
    double duration_time_ = 0.0;

    FILE *fp_yuv_;
    std::string save_name_;

    std::chrono::time_point<std::chrono::steady_clock> start_time_;
    std::chrono::time_point<std::chrono::steady_clock> end_time_;
    std::chrono::time_point<std::chrono::steady_clock> step_start_time_;
    std::chrono::time_point<std::chrono::steady_clock> step_end_time_;
    std::chrono::duration<double, std::milli>          step_diff_;
};

#endif //_DECODE_H
