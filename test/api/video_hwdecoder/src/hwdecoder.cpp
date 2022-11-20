#include "hwdecoder.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "time.h"

static enum AVPixelFormat hw_pix_fmt;
static enum AVPixelFormat get_hw_format(AVCodecContext *ctx,
                                        const enum AVPixelFormat *kPixFmts) {
  const enum AVPixelFormat *k_format;

  for (k_format = kPixFmts; *k_format != -1; k_format++) {
    if (*k_format == hw_pix_fmt) return *k_format;
  }

  printf("failed to get HW surface format.\n");
  return AV_PIX_FMT_NONE;
}

int Decoder::decode_process(std::string source, int dev_id, int thread_id,
                     bool is_dump) {
  int ret = -1;
  if (dev_id) dev_id_ = dev_id;
  else        dev_id_ = 0;

  thread_id_     = thread_id;
  duration_time_ = 3000.0;

  if ((ret = init_decode(source)) < 0) {
    printf("init decode failed, ret:%d\n", ret);
    return -1;
  }

  if (is_dump) {
    save_name_ = "out_video_" + std::to_string(thread_id) + ".yuv";
    fp_yuv_    = fopen(save_name_.c_str(), "wb+");
  }
  is_dump_file_ = is_dump;

  start_time_ = end_time_           = std::chrono::steady_clock::now();
  step_start_time_ = step_end_time_ = std::chrono::steady_clock::now();

  if ((ret = decoding()) < 0) {
    printf("decoding failed, ret:%d\n", ret);
    return -1;
  }
  end_time_ = std::chrono::steady_clock::now();

  close_decode();

  return 0;
}

int Decoder::decode_hwdevice_ctx_init() {
  int err = 0;
  snprintf(dev_idx_des, sizeof(dev_id_), "%d", dev_id_);
  if ((err = av_hwdevice_ctx_create(
       &hw_device_ctx_, k_type_, dev_idx_des, NULL, 0)) < 0) {
    printf("failed to create specified HW device.\n");
    return err;
  }
  codec_ctx_->hw_device_ctx = av_buffer_ref(hw_device_ctx_);

  return err;
}

int Decoder::init_decode(std::string source) {
  AVDictionary *dec_opts = NULL;
  AVStream     *video    = NULL;
  AVDictionary *options  = NULL;
  AVCodec *codec         = NULL;

  int ret = -1;
  if ((ret = avformat_network_init()) != 0) {
    printf("avformat_network_init failed, ret: %d\n", ret);
    return ret;
  }

  k_type_ = av_hwdevice_find_type_by_name("mlu");

  if (k_type_ == AV_HWDEVICE_TYPE_NONE) {
    printf("device type %s is not supported.\n", "mlu");
    printf("available device types:");
    while ((k_type_ = av_hwdevice_iterate_types(k_type_)) !=
           AV_HWDEVICE_TYPE_NONE)
      printf(" %s\n", av_hwdevice_get_type_name(k_type_));
    return -1;
  }

  format_ctx_ = avformat_alloc_context();
  if (!strncmp(source.c_str(), "rtsp", 4)
      || !strncmp(source.c_str(), "rtmp", 4)) {
    av_dict_set(&options, "buffer_size", "1024000", 0);
    av_dict_set(&options, "max_delay",   "500000", 0);
    av_dict_set(&options, "stimeout",    "20000000", 0);
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
  } else {
    av_dict_set(&options, "stimeout", "20000000", 0);
  }

  ret = avformat_open_input(&format_ctx_, source.c_str(), NULL, &options);
  if (ret != 0) {
    av_dict_free(&options);
    printf("avformat_open_input failed, ret: %d\n", ret);
    return ret;
  }
  av_dict_free(&options);
  ret = avformat_find_stream_info(format_ctx_, NULL);
  if (ret < 0) {
    printf("avformat_find_stream_info failed, ret: %d\n", ret);
    return ret;
  }
  if ((ret = find_video_stream_index()) < 0) {
    printf("find_video_stream_index failed, ret: %d\n", ret);
    return ret;
  }

  video = format_ctx_->streams[video_stream_];
  switch (video->codecpar->codec_id) {
    case AV_CODEC_ID_H264:
      codec = avcodec_find_decoder_by_name("h264_mludec"); break;
    case AV_CODEC_ID_HEVC:
      codec = avcodec_find_decoder_by_name("hevc_mludec"); break;
    case AV_CODEC_ID_VP8:
      codec = avcodec_find_decoder_by_name("vp8_mludec"); break;
    case AV_CODEC_ID_VP9:
      codec = avcodec_find_decoder_by_name("vp9_mludec"); break;
    case AV_CODEC_ID_MJPEG:
      codec = avcodec_find_decoder_by_name("mjpeg_mludec"); break;
    default:
      codec = avcodec_find_decoder(video->codecpar->codec_id); break;
  }
  if (codec == NULL) {
    printf("could not find codec!\n");
    return -1;
  }

  for (int i = 0;; i++) {
    const AVCodecHWConfig *k_config = avcodec_get_hw_config(codec, i);
    if (!k_config) {
      fprintf(stderr, "decode %s not support device type %s.\n",
              codec->name, av_hwdevice_get_type_name(k_type_));
      return -1;
    }
    if (k_config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
        k_config->device_type == k_type_) {
      hw_pix_fmt = k_config->pix_fmt;
      break;
    }
  }

  codec_ctx_ = avcodec_alloc_context3(codec);
  if (avcodec_parameters_to_context(codec_ctx_, video->codecpar) != 0) {
    printf("could not copy codec context: %d\n", ret);
    return -1;
  }

  codec_ctx_->get_format = get_hw_format;

  av_dict_set_int(&dec_opts, "device_id", dev_id_, 0);

  if (decode_hwdevice_ctx_init() < 0) {
    printf("hw decode device init failed\n");
    return -1;
  }

  // codec_ctx_->thread_count = 1;
  if (avcodec_open2(codec_ctx_, codec, &dec_opts) < 0) {
    printf("could not open codec: %d\n", ret);
    return -1;
  }
  av_dict_free(&dec_opts);

  frame_ = av_frame_alloc();
  img_width_ = codec_ctx_->width;
  img_height_ = codec_ctx_->height;

  printf("[Source]:%s, video stream idx:%d, width:%d, height:%d\n",
         source.c_str(), video_stream_, img_width_, img_height_);
  return 0;
}

int Decoder::find_video_stream_index() {
  for (size_t i = 0; i < format_ctx_->nb_streams; i++) {
    if (format_ctx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      video_stream_ = i;
      return 0;
    }
  }
  return -1;
}

int Decoder::decoding() {
  int ret = 0;
  AVPacket packet = {0};
  AVFrame *sw_frame = NULL;
  AVFrame *tmp_frame = NULL;
  av_init_packet(&packet);

  while (1) {
    ret = av_read_frame(format_ctx_, &packet);
    if (packet.stream_index != video_stream_) {
      av_packet_unref(&packet);
      continue;
    }
    ret = avcodec_send_packet(codec_ctx_, &packet);
    if (ret < 0 && ret != AVERROR_EOF) {
      av_packet_unref(&packet);
      if (ret == AVERROR(EAGAIN)) continue;
      printf("send packet failed, ret:%d\n", ret);
      return ret;
    }
    while (ret >= 0) {
      ret = avcodec_receive_frame(codec_ctx_, frame_);
      if (ret < 0 && ret != AVERROR_EOF) {
        if (ret != AVERROR(EAGAIN)) return ret;
      } else if (ret == 0) {
        if (is_dump_file_) {
          sw_frame = av_frame_alloc();
          if (frame_->format == hw_pix_fmt) {
            if ((ret = av_hwframe_transfer_data(sw_frame, frame_, 0)) < 0) {
              av_frame_free(&sw_frame);
              printf("download mlu memery data to cpu\n");
              return AVERROR_EOF;
            }
            tmp_frame = sw_frame;
          } else {
            tmp_frame = frame_;
          }
          save_yuv_file(tmp_frame);
          av_frame_free(&sw_frame);
        }
        decode_frame_num_++;
        av_frame_unref(frame_);
        print_perf_step();
      } else if (ret == AVERROR_EOF) {
        return 0;
      }
    }
    av_packet_unref(&packet);
  }
  return 0;
}

void Decoder::save_yuv_file(AVFrame *p_frame) {
  fwrite(p_frame->data[0], 1, img_width_ * img_height_, fp_yuv_);     // Y
  fwrite(p_frame->data[1], 1, img_width_ * img_height_ / 2, fp_yuv_); // UV
}

void Decoder::close_decode() {
  if (is_dump_file_) fclose(fp_yuv_);
  if (frame_) av_frame_free(&frame_);
  avcodec_free_context(&codec_ctx_);
  avformat_close_input(&format_ctx_);
  if (hw_device_ctx_)
    av_buffer_unref(&hw_device_ctx_);
}

void Decoder::print_perf_total() {
  std::chrono::duration<double, std::milli> diff = end_time_ - start_time_;
  actual_fps_ = 0.0f;
  if (diff.count()) {
    actual_fps_ = (decode_frame_num_ * 1000 * 1.f / diff.count());
  }

  printf("[chan_%d_perf] fps:%.2f, frame cnt:%d, dec latency:%.2fms\n",
    thread_id_, actual_fps_, decode_frame_num_, diff.count()/(1.f*decode_frame_num_));
}

void Decoder::print_perf_step() {
  double step_fps = 0.0f;
  step_dec_frame_num_++;
  step_end_time_ = std::chrono::steady_clock::now();
  step_diff_ = step_end_time_ - step_start_time_;
  if (step_diff_.count() > duration_time_) {
    step_fps = (step_dec_frame_num_ * 1000 * 1.f / step_diff_.count());
    printf("[stepFpsStats] chan:%.3d, fps:%.2f, step time:%.2fms, ",
          thread_id_, step_fps, step_diff_.count());
    printf("step count:%d, frame count:%d\n", step_dec_frame_num_,
          decode_frame_num_);
    step_dec_frame_num_ = 0;
    step_start_time_ = std::chrono::steady_clock::now();
  }
}

double Decoder::get_actual_fps() {
  if (actual_fps_ < 0) {
    actual_fps_ = 0.0f;
  }
  return actual_fps_;
}
