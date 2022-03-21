#ifndef _TRANSCODE_H
#define _TRANSCODE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avstring.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#ifdef __cplusplus
};
#endif

#define DEFAULT_BPS   0x100000
#define DEFAULT_GOP   50
#define DEFAULT_QMIN  26
#define DEFAULT_QMAX  51
#define DEFAULT_B_NUM 1

class Transcode {
public:
    int analysis_txt_file(std::string);
    void filter(std::string, std::string, int, int, int, int, int);
    int init_decoder(std::string, int);
    void print_perf_step();
    double get_actual_fps();
    std::string get_output_name();

private:
    int init_filter(std::string);
    void close_filter();
    int init_encoder(std::string, int, int, int);
    int encode_video_frame(AVFrame *);
    int filter_write_video_frame(AVFrame *);
    int filtering();

private:
    int video_stream = -1;
    int stream_index = 0;
    int64_t last_pts = AV_NOPTS_VALUE;
    AVFormatContext *p_format_ctx = NULL;
    AVFilterGraph *filter_graph   = NULL;
    AVFormatContext *ifmt_ctx     = NULL;
    AVFormatContext *ofmt_ctx     = NULL;
    AVCodecContext *decoder_ctx   = NULL;
    AVCodecContext *encoder_ctx   = NULL;
    AVFilterContext *buffersink_ctx = NULL;
    AVFilterContext *buffersrc_ctx  = NULL;

    AVFilterInOut *outputs = NULL;
    AVFilterInOut *inputs  = NULL;
    AVDictionary *d_avdict = NULL;
    AVDictionary *e_avdict = NULL;
    AVPacket *d_packet     = NULL;
    AVPacket *e_packet     = NULL;

    AVFrame *p_frame = NULL;
    // AVFrame *filt_frame = NULL;

	int dev_id_;
    int thread_id_;
    int infer_time_;
    int write_flag_;
    int img_width_;
    int img_height_;

public:
    int infer_frame_num_      = 0;
    int step_infer_frame_num_ = 0;
    double actual_fps_       = 0.0;
    double duration_time_    = 0.0;
    double thread_fps_       = 0.0;
    double decode_frame_num_ = 0.0;

    std::chrono::time_point<std::chrono::steady_clock> step_start_time;
    std::chrono::time_point<std::chrono::steady_clock> step_end_time;
    std::chrono::duration<double, std::milli> step_diff;
};

#endif //_TRANSCODE_H
