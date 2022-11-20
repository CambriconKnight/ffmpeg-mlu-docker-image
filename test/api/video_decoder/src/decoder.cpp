#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "decoder.h"
#include "time.h"

#define PRINT_PERF 1

#define CHECKFFRET(ret) \
if (ret < 0) {\
    av_log(nullptr, ret != AVERROR(EAGAIN) ? (ret != AVERROR_EOF ? \
        AV_LOG_ERROR : AV_LOG_INFO) : AV_LOG_DEBUG, "%s %d : %d\n", __FILE__, __LINE__, ret);\
    return ret;\
}

using namespace std;
int Decoder::decode_process(std::string source, int dev_id, int th_id, bool is_dump) {
    int ret;

    if (dev_id_ >= 0) dev_id_ = dev_id;
    thread_id_     = th_id;
    duration_time_ = 3000.0;
    is_dump_file_  = is_dump;

    if ((ret = init_decode(source)) < 0) {
        printf("init decode failed, ret:%d\n", ret);
        return -1;
    }

    if (is_dump_file_) {
        save_name_ = "save_video_yuv_" + std::to_string(thread_id_) + ".yuv";
        fp_yuv_ = fopen(save_name_.c_str(), "wb+");
    }

    start_time_ = end_time_ = std::chrono::steady_clock::now();
    step_start_time_ = step_end_time_ = std::chrono::steady_clock::now();

    if ((ret = decoding()) < 0) {
        printf("decoding failed, ret:%d\n", ret);
        return -1;
    }

    end_time_ = std::chrono::steady_clock::now();

    close_decode();
    return 0;
}

int Decoder::init_decode(std::string source) {
    AVStream     *video    = nullptr;
    AVCodec      *p_codec  = nullptr;
    AVDictionary *options  = nullptr;
    AVDictionary *dec_opts = nullptr;

    int ret = -1;
    if ((ret = avformat_network_init()) != 0) {
        printf("avformat_network_init failed, ret(%d)\n", ret);
        return ret;
    }

    p_format_ctx_ = avformat_alloc_context();
    if (!strncmp(source.c_str(), "rtsp", 4) || !strncmp(source.c_str(), "rtmp", 4)) {
        printf("decode rtsp/rtmp stream\n");
        av_dict_set(&options, "buffer_size",    "1024000", 0);
        av_dict_set(&options, "max_delay",      "500000", 0);
        av_dict_set(&options, "stimeout",       "20000000", 0);
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
    } else {
        printf("decode local file stream\n");
        av_dict_set(&options, "stimeout", "20000000", 0);
        av_dict_set(&options, "vsync", "0", 0);
    }

    ret = avformat_open_input(&p_format_ctx_, source.c_str(), nullptr, &options);
    av_dict_free(&options);
    if (ret != 0) {
        printf("avformat_open_input failed, ret(%d)\n",ret);
        return ret;
    }
    ret = avformat_find_stream_info(p_format_ctx_, nullptr);
    if (ret < 0) {
        printf("avformat_find_stream_info failed, ret(%d)\n", ret);
        return ret;
    }
    if ((ret = find_video_stream_index()) < 0) {
        printf("find_video_stream_index failed, ret(%d)\n", ret);
        return ret;
    }

    video = p_format_ctx_->streams[video_stream_];
    switch(video->codecpar->codec_id) {
    case AV_CODEC_ID_H264:
        p_codec = avcodec_find_decoder_by_name("h264_mludec");
        break;
    case AV_CODEC_ID_HEVC:
        p_codec = avcodec_find_decoder_by_name("hevc_mludec");
        break;
    case AV_CODEC_ID_VP8:
        p_codec = avcodec_find_decoder_by_name("vp8_mludec");
        break;
    case AV_CODEC_ID_VP9:
        p_codec = avcodec_find_decoder_by_name("vp9_mludec");
        break;
    case AV_CODEC_ID_MJPEG:
        p_codec = avcodec_find_decoder_by_name("mjpeg_mludec");
        break;
    default:
        p_codec = avcodec_find_decoder(video->codecpar->codec_id);
        break;
    }
    if(p_codec == nullptr) {
        printf("Unsupported codec! \n");
        return -1;
    }

    p_codec_ctx_ = avcodec_alloc_context3(p_codec);
    if(avcodec_parameters_to_context(p_codec_ctx_, video->codecpar) != 0) {
        printf("Could not copy codec context, ret(%d)\n", ret);
        return -1;
    }

    av_dict_set_int(&dec_opts, "device_id", dev_id_, 0);
    // av_dict_set_int(&dec_opts, "trace", 1, 0);
    // p_codec_ctx_->flags |= AV_CODEC_FLAG_LOW_DELAY;
    // Notice: This 'flags' is for set low delay decode. When decoding get blocked, try to uncomment the 'flags'.

    // p_codec_ctx_->thread_count = 1;
    if(avcodec_open2(p_codec_ctx_, p_codec, &dec_opts) < 0) {
        printf("Could not open codec, ret(%d)\n", ret);
        return -1;
    }
    av_dict_free(&dec_opts);

    p_frame_=av_frame_alloc();
    img_width_ = p_codec_ctx_->width;
    img_height_ = p_codec_ctx_->height;

    printf("[source]:%s, video stream idx:%d, width:%d, height:%d\n",
        source.c_str(), video_stream_, img_width_, img_height_);

    return 0;
}

int Decoder::find_video_stream_index() {
    for (size_t i = 0; i < p_format_ctx_->nb_streams; i++) {
        if (p_format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_ = i;
            return 0;
        }
    }
    return -1;
}

int Decoder::decoding() {
    int ret;
    AVPacket packet;
    av_init_packet(&packet);
    while(1) {
        ret = av_read_frame(p_format_ctx_, &packet);
        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                // printf("[thread:%d] av_read_frame ret eof \n", thread_id_);
            } else {
                printf("av_read_frame failed, ret(%d)\n", ret);
                return -1;
            }
        }
        if (packet.stream_index != video_stream_) {
            av_packet_unref(&packet);
            continue;
        }
        ret = avcodec_send_packet(p_codec_ctx_, &packet);
        CHECKFFRET(ret);

        while (ret >= 0) {
            ret = avcodec_receive_frame(p_codec_ctx_, p_frame_);
            if (ret < 0 && ret != AVERROR_EOF) {
                if (ret != AVERROR(EAGAIN))
                    return ret;
            } else if (ret == 0) {
                if(is_dump_file_){
                    save_yuv_file(p_frame_);
                }
                dec_num_++;
                av_frame_unref(p_frame_);
                print_perf_step();
            } else if (ret == AVERROR_EOF) {
                return 0;
            }
        }
        av_packet_unref(&packet);
    }

    return 0;
}

void  Decoder::save_yuv_file(AVFrame *p_frame) {
    fwrite(p_frame->data[0], 1, img_width_ * img_height_, fp_yuv_);     // Y
    fwrite(p_frame->data[1], 1, img_width_ * img_height_ / 2, fp_yuv_); // UV
}

void Decoder::close_decode() {
    if (is_dump_file_) fclose(fp_yuv_);
    if (p_frame_) av_frame_free(&p_frame_);
    avformat_close_input(&p_format_ctx_);
    avcodec_free_context(&p_codec_ctx_);
}

void Decoder::print_perf_total() {
    std::chrono::duration<double, std::milli> diff = end_time_ - start_time_;
    actual_fps_ = 0.0f;
    if (diff.count()) {
        actual_fps_ =  (dec_num_ * 1000 * 1.f / diff.count());
    }

    printf("[chan_%d_perf] fps:%.2f, frame cnt:%d, dec latency:%.2fms\n",
        thread_id_, actual_fps_, dec_num_, diff.count() / (1.f * dec_num_));
}

void Decoder::print_perf_step() {
  double step_fps = 0.0f;
  step_dec_num_++;
  step_end_time_ = std::chrono::steady_clock::now();
  step_diff_ = step_end_time_ - step_start_time_;
  if (step_diff_.count() > duration_time_) {
    step_fps = (step_dec_num_ * 1000 * 1.f / step_diff_.count());
    printf("[stepFpsStats] chan:%.3d, fps:%.2f, step time:%.2fms, ",
          thread_id_, step_fps, step_diff_.count());
    printf("step count:%d, frame count:%d\n", step_dec_num_,
          dec_num_);
    step_dec_num_ = 0;
    step_start_time_ = std::chrono::steady_clock::now();
  }
}

double Decoder::get_actual_fps() {
    if (actual_fps_ < 0) actual_fps_ = 0.0f;
    return actual_fps_;
}
