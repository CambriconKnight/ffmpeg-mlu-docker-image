#ifndef _VIDEO_ENCODER_H
#define _VIDEO_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#include <vector>
#include <string>
#include <unordered_map>

typedef struct enc_params {
    std::string bit_rate;
    std::string b_frame;
    std::string gop_size;
    std::string rc_mode;
    std::string profile;
    std::string level;
    std::string quality;
    std::string q_min;
    std::string q_max;
    std::string inputfile;
    std::string enc_type;
    std::string enc_w;
    std::string enc_h;
    std::string enc_num;
    std::string dev_id;
    std::string save_flag;
} enc_params_t;

typedef struct EncoderContext{
    AVCodec *codec;
    AVStream *videoStream;
    AVCodecContext *codecContext;
    AVFormatContext *formatContext;
} EncoderContext;

class VideoEncoder {
public:
    int init_encoder(enc_params_t enc_params, int threadid);
    int do_encode();
    void close_encoder();
    void print_perf_step();
    int img_write(AVPacket *e_pkt, int img_idx);

private:

    EncoderContext enc_ctx;
    AVPacket       *e_pkt;
    AVFrame        *p_frame;

    int chan_id = 0;
    int need_enc_count = 0;
    int step_enc_num = 0;
	int dev_id = 0;
    int write_flag = 0;
    bool jpeg_enc_type;

    FILE* input_file;
    FILE* output_file;

    double duration_time = 0.0;
    std::chrono::time_point<std::chrono::steady_clock> step_start_time;
    std::chrono::time_point<std::chrono::steady_clock> step_end_time;
    std::chrono::duration<double, std::milli> step_diff;

public:
    int enc_frame_num = 0;
    std::chrono::duration<double, std::milli> enc_time;
    std::chrono::time_point<std::chrono::steady_clock> enc_start_time;
    std::chrono::time_point<std::chrono::steady_clock> enc_end_time;

};

#endif //_VIDEO_ENCODER_H
