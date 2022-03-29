#ifndef _VIDEO_TRANSCODER_H
#define _VIDEO_TRANSCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/avstring.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#ifdef __cplusplus
}
#endif

#include <vector>
#include <string>

#define DEFAULT_BPS 0x100000
#define DEFAULT_GOP 50
#define DEFAULT_QMIN 26
#define DEFAULT_QMAX 51
#define DEFAULT_B_NUM 1

typedef struct TranscoderContext{
    AVFormatContext *ifmt_ctx;
    AVFormatContext *ofmt_ctx;
    AVCodecContext *decoder_ctx;
    AVCodecContext *encoder_ctx;
    AVCodec *decoder_codec;
    AVCodec *encoder_codec;
    AVStream *ost;
} TranscoderContext;

class VideoTranscoder {

public:
    int initTranscoder(std::string source, int dst_w, int dst_h, int device_id, int threadid, int strm_write_flag);
    int transcode();
    void closeTranscoder();
    void printPerf();
    double getActualFps();
    void printPerfStep();
    
private:
    std::string getOutputName();
    int streamWrite(std::string);

private:

    int videoStream = -1;
    AVDictionary *dAVdict = nullptr;
    TranscoderContext ctx;
    AVDictionary *eAVdict = nullptr;
    SwsContext *sws_ctx = nullptr;
    AVPacket *dPacket;
    AVPacket *ePacket;

    AVFrame *pFrame = nullptr;
    uint8_t *buffer = nullptr;

    int enc_ret = 0;
    int dec_ret = 0;

    int imgWidth = 0;
    int imgHeight = 0;
    int imgSize = 0;

    int threadId = 0;
    int transcodeCount = 0;
    int transcodeFrameNum = 0;
    int stepTranscodeFrameNum = 0;
	  int devId;
    int write_flag = 0;

    FILE* input_file;

    double transcodeFps = 0.0f;
    double durationTime = 0.0;

    int encoder_initialized = 0;
    uint64_t start_time_ts = 0;
    uint64_t transcode_time = 0;
    uint64_t step_start_time_ts = 0;
    uint64_t step_diff = 0;
    
};

#endif //_VIDEO_ENCODER_H