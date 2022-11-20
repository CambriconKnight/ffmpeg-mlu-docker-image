#ifndef _DECODER_H
#define _DECODER_H
#include <chrono>

#include "stdint.h"
#include "stdio.h"
#include "string"
#include "unistd.h"
#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avassert.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

/**
 * This class implements the flow of hardware accelerated video decoding and
 * defines variables and Pointers to important structures to be used during
 * hardware decoding.
 *
 *   Decoder *d = new Decoder();
 *   d->decode_process(filename, dev_id, thread_id, dump_flag);
 *   d->print_perf_total();
 *   d->get_actual_fps();
 *   delete d;
 */
class Decoder {
 public:
  /**
   * Provides the interface for the whole process of hardware accelerated
   * decoding
   *
   * @param source original media file to be decoded. Video formats
   *               mp4/mkv/avi/h264/hevc and rtsp/rtmp streams are supported.
   * @param dev_id  Device id.
   *                determined by the number of devices on different servers
   * @param thread_id number of threads
   * @param is_dump whether to save decoded data that pixel format is yuv.
   * @return 0 if ok
   */
  int decode_process(std::string source, int dev_id, int thread_id, bool is_dump);
  /**
   * Calculates and prints the FPS for each thread
   */
  void print_perf_total();
  /**
   * Calculates and prints the FPS for each thread per durations
   * @note duration is a global variable, initial value is 5000.0
   */
  void print_perf_step();
  /**
   * Returns actual FPS.
   *
   * @return actual_fps_ if >= 0, 0.0f on error
   */
  double get_actual_fps();

 private:
  /**
   * Frees memory allocated manually during decoding.
   *
   * @note av_frame_free();
   *       avcodec_close();
   *       avcodec_free_context();
   *       avformat_close_input();
   */
  void close_decode();
  /**
   * Initialization of the decoder. Compared with traditional decoder
   * initialization, hardware acceleration related initialization is added to
   * the process
   *
   * @param source original media file to be decoded. Video formats
   *               mp4/mkv/avi/h264/hevc and rtsp/rtmp streams are supported.
   *
   * @return >=0 if OK, AVERROR_xxx or < 0 on error
   */
  int init_decode(std::string source);
  /**
   * Obtain the hardware acceleration device context by device number and
   * hardware acceleration type. By calling function av_hwdevice_ctx_create and
   * av_buffer_ref() to initialize the hardware acceleration environment
   *
   * @return >=0 if OK, AVERROR_xxx on error
   */
  int decode_hwdevice_ctx_init();
  /**
   * Query the sequence number of the video stream whose data stream type is
   * AVMEDIA_TYPE_VIDEO. There is a member variable named video_stream_, when
   * find AVMEDIA_TYPE_VIDEO, assign serial number to this variable.
   *
   * @return 0 if OK, -1 on error
   */
  int find_video_stream_index();
  /**
   * Hardware-accelerated video decoding.
   *
   * @return >=0 if OK, <0 on error
   */
  int decoding();
  /**
   * Save decoded YUV data.
   *
   * @param p_frame decoded video frame, pixel format supports
   *                nv12/nv21/p010/yuv420p, set nv12 by default.
   */
  void save_yuv_file(AVFrame *p_frame);

 private:
  // Uniquely identifies the hardware acceleration device type.
  // Get by function av_hwdevice_find_type_by_name.
  enum AVHWDeviceType k_type_;
  AVCodecContext *codec_ctx_   = NULL;
  AVBufferRef *hw_device_ctx_  = NULL;
  AVFormatContext *format_ctx_ = NULL;
  AVFrame *frame_              = NULL;

  int img_width_          = 0;
  int img_height_         = 0;
  int video_stream_       = -1;
  int decode_frame_num_   = 0;
  int step_dec_frame_num_ = 0;
  double actual_fps_      = 0.0;
  double duration_time_   = 0.0;

  int dev_id_         = 0;
  int thread_id_      = 0;
  char dev_idx_des[4] = {'\0'};
  bool is_dump_file_  = false;

  FILE *fp_yuv_;
  std::string save_name_;

  // Used to detect decoding performance.
  std::chrono::time_point<std::chrono::steady_clock> start_time_;
  std::chrono::time_point<std::chrono::steady_clock> end_time_;
  std::chrono::time_point<std::chrono::steady_clock> step_start_time_;
  std::chrono::time_point<std::chrono::steady_clock> step_end_time_;
  std::chrono::duration<double, std::milli> step_diff_;
  std::chrono::duration<double, std::milli> save_time_diff_;
};

#endif  // _DECODER_H
