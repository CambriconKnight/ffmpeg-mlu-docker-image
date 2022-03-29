#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include "vid2vid_trans.h"
#include "time.h"
#include <vector>


#define CHECKFFRET(ret) \
if (ret < 0)\
{\
    av_log(nullptr, ret != AVERROR(EAGAIN) ? (ret != AVERROR_EOF ? AV_LOG_ERROR : AV_LOG_INFO) : AV_LOG_DEBUG, "%s %d : %d\n", __FILE__, __LINE__, ret);\
    return ret;\
}

using namespace std;

int VideoTranscoder::initTranscoder(std::string source, int dst_w, int dst_h, int device_id, int threadid, int strm_write_flag) {
    write_flag = strm_write_flag;
    devId = device_id;
    threadId = threadid;
    int ret;
    AVStream *video = NULL;
    if ((ret = avformat_network_init()) != 0) {
        cout << "avformat_network_init failed, ret: " << ret << endl;
        return ret;
    }

    /*---------init decoder-----------*/
    ctx.ifmt_ctx = avformat_alloc_context();
    if (!strncmp(source.c_str(), "rtsp", 4) || !strncmp(source.c_str(), "rtmp", 4)) {
        std::cout << "decode rtsp/rtmp stream " << std::endl;
        av_dict_set(&dAVdict, "buffer_size", "1024000", 0);
        av_dict_set(&dAVdict, "max_delay", "500000", 0);
        av_dict_set(&dAVdict, "stimeout", "20000000", 0);
        av_dict_set(&dAVdict, "rtsp_transport", "tcp", 0);
    } else {
        std::cout << "decode local file stream " << std::endl;
        av_dict_set(&dAVdict, "stimeout", "20000000", 0);
    }

    if ((ret = avformat_open_input(&ctx.ifmt_ctx, source.c_str(), NULL, NULL)) < 0) {
        fprintf(stderr, "Cannot open input file '%s', Error code: %d\n",source.c_str(), ret);
        return ret;
    }

    if ((ret = avformat_find_stream_info(ctx.ifmt_ctx, NULL)) < 0) {
        fprintf(stderr, "Cannot find input stream information. Error code: %d\n", ret);
        return ret;
    }

    ret = av_find_best_stream(ctx.ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "Cannot find a video stream in the input file. "
                "Error code: %d\n", ret);
        return ret;
    }
    videoStream = ret;
    ctx.decoder_codec = avcodec_find_decoder_by_name("h264_mludec");
    if( !ctx.decoder_codec ) { 
        fprintf(stderr,"decoder find failed."); 
        return AVERROR(ENODEV);
    }
    if (!(ctx.decoder_ctx = avcodec_alloc_context3(ctx.decoder_codec)))
        return AVERROR(ENOMEM);

    video = ctx.ifmt_ctx->streams[videoStream];
    if ((ret = avcodec_parameters_to_context(ctx.decoder_ctx, video->codecpar)) < 0) {
        fprintf(stderr, "avcodec_parameters_to_context error. Error code: %d\n", ret);
        return ret;
    }

    string resize_str;
    resize_str = to_string(dst_w) + "x" + to_string(dst_h);
    std::cout << "resize :" << resize_str << std::endl;
    if (av_stristr(ctx.decoder_codec->name,"mludec")) {
        // ctx.decoder_ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        // av_dict_set(&dAVdict, "resize", "352x288", 0);
        av_dict_set(&dAVdict, "resize", resize_str.c_str(), 0);
        // av_dict_set_int(&dAVdict, "trace", 1, 0);
        av_dict_set_int(&dAVdict, "device_id", devId, 0);
    }
    if ((ret = avcodec_open2(ctx.decoder_ctx, ctx.decoder_codec, &dAVdict)) < 0) {
        fprintf(stderr, "Failed to open codec for decoding. Error code: %d\n", ret);
        return ret;
    }
    if (dAVdict) {
        av_dict_free(&dAVdict);
        dAVdict = NULL;
    }

    /*----------init encoder---------*/
    if ((ret = (avformat_alloc_output_context2(&(ctx.ofmt_ctx), NULL, NULL, getOutputName().c_str()))) < 0) {
        fprintf(stderr, "Failed to deduce output format from file extension. Error code: %d\n", ret);
        return ret;
    }
    ctx.encoder_codec = avcodec_find_encoder_by_name("h264_mluenc");
    if( !ctx.encoder_codec ) { 
        fprintf(stderr,"encoder find failed."); 
        return AVERROR(ENODEV);
    }
    if (!(ctx.encoder_ctx = avcodec_alloc_context3(ctx.encoder_codec))) {
        return AVERROR(ENOMEM);
    }
    if (write_flag) {
        if ((ret = (avformat_alloc_output_context2(&(ctx.ofmt_ctx), NULL, NULL, getOutputName().c_str()))) < 0) {
            fprintf(stderr, "Failed to deduce output format from file extension. Error code: %d\n", ret);
            return ret;
        }
        ret = avio_open(&(ctx.ofmt_ctx->pb), getOutputName().c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Cannot open output file. Error code: %d\n", ret);
            return ret;
        }
    }
    ctx.encoder_ctx->time_base = (AVRational){1, 25};
    /* take first format from list of supported formats */
    if (ctx.encoder_codec->pix_fmts)
        ctx.encoder_ctx->pix_fmt = ctx.encoder_codec->pix_fmts[0];
    else
        ctx.encoder_ctx->pix_fmt = ctx.decoder_ctx->pix_fmt;

    // ctx.encoder_ctx->width     = ctx.decoder_ctx->width;
    // ctx.encoder_ctx->height    = ctx.decoder_ctx->height;
    ctx.encoder_ctx->width     = dst_w;
    ctx.encoder_ctx->height    = dst_h;

    ctx.encoder_ctx->bit_rate = DEFAULT_BPS;
    ctx.encoder_ctx->gop_size = DEFAULT_GOP;
    ctx.encoder_ctx->sample_aspect_ratio = ctx.decoder_ctx->sample_aspect_ratio;
    ctx.encoder_ctx->framerate =  (AVRational){25, 1};
    // ctx.encoder_ctx->max_b_frames = 3;
    ctx.encoder_ctx->qmin = DEFAULT_QMIN;
    ctx.encoder_ctx->qmax = DEFAULT_QMAX;
    if (av_stristr(ctx.encoder_codec->name,"mluenc")) {
        //av_dict_set_int(&eAVdict, "trace", 1, 0);
        av_dict_set_int(&eAVdict, "device_id", devId, 0);
    }
    if ((ret = avcodec_open2(ctx.encoder_ctx, ctx.encoder_codec, &eAVdict)) < 0) {
        fprintf(stderr, "Failed to open encode codec. Error code: %d\n", ret );
        return ret;
    }
    if (eAVdict) {
        av_dict_free(&eAVdict);
        eAVdict = NULL;
    }
    if (!(ctx.ost = avformat_new_stream(ctx.ofmt_ctx, ctx.encoder_codec))) {
        fprintf(stderr, "Failed to allocate stream for output format.\n");
        return AVERROR(ENOMEM);
    }

    ctx.ost->time_base = ctx.encoder_ctx->time_base;
    ret = avcodec_parameters_from_context(ctx.ost->codecpar, ctx.encoder_ctx);
    if (ret < 0) {
        fprintf(stderr, "Failed to copy the stream parameters. "
                "Error code: %d\n", ret);
        return ret;
    }
    if (write_flag) {
        /* write the stream header */
        if ((ret = avformat_write_header(ctx.ofmt_ctx, NULL)) < 0) {
            fprintf(stderr, "Error while writing stream header. "
                    "Error code: %d\n", ret);
            return ret;
        }
    }

    return ret;
}

int VideoTranscoder::transcode() {
    /*----------init data---------*/
    int is_get_eof = 0;
    ePacket = av_packet_alloc();
    av_init_packet(ePacket);
    dPacket = av_packet_alloc();
    av_init_packet(dPacket);

    durationTime = 5000.0;
    start_time_ts = step_start_time_ts = av_gettime_relative();

    while (dec_ret >= 0 || dec_ret == AVERROR(EAGAIN)) {
        dec_ret = av_read_frame(ctx.ifmt_ctx, dPacket);
        if (videoStream != dPacket->stream_index)
            continue;
        if (dec_ret < 0) {
            if (dec_ret == AVERROR_EOF) {
                // fprintf(stdout, "[thread: %d] -- av_read_frame ret eof!\n", threadId);
                is_get_eof = 1;
            } else {
                if (dPacket) av_packet_free(&dPacket);
                fprintf(stderr, "av_read_frame failed, ret: %d\n", dec_ret);
                return -1;
            }
        }
        if (is_get_eof) {
            dPacket->data = NULL;
            dPacket->size = 0;
        }
        dec_ret = avcodec_send_packet(ctx.decoder_ctx, dPacket);
        if (dec_ret < 0) {
            if (dec_ret != AVERROR_EOF) {
                if (dPacket) av_packet_free(&dPacket);
                fprintf(stderr, "Error during decoding. Error code: %d\n", dec_ret);
                return dec_ret;
            }
        }

        while (dec_ret >= 0) {
            if ( !(pFrame = av_frame_alloc())) {
                return AVERROR(ENOMEM);
            }
            dec_ret = avcodec_receive_frame(ctx.decoder_ctx, pFrame);
            // if (dec_ret == AVERROR(EAGAIN) || dec_ret == AVERROR_EOF) {
            if (dec_ret < 0 && dec_ret != AVERROR_EOF) {
                // fprintf(stderr, "Error while decoding. Error code: %d\n", dec_ret);
                break;
            } else if (dec_ret ==0 || dec_ret == AVERROR_EOF) {
                if (dec_ret != AVERROR_EOF) {
                    transcodeFrameNum++;
                    stepTranscodeFrameNum++;
                }
               if (dec_ret == AVERROR_EOF) {
                   pFrame = NULL;
               }
                enc_ret = avcodec_send_frame(ctx.encoder_ctx, pFrame);
                if (enc_ret < 0 && enc_ret != AVERROR_EOF) {
                  fprintf(stderr, "Error during encoding. Error code: %d\n", enc_ret);
                  goto fail;
                }
                while (enc_ret >= 0) {
                    enc_ret = avcodec_receive_packet(ctx.encoder_ctx, ePacket);
                    if (enc_ret == AVERROR_EOF) {
                        transcode_time = av_gettime_relative() - start_time_ts;
                        av_write_trailer(ctx.ofmt_ctx);
                        std::cout << "[thread:" << threadId <<"] encoder receive eof" << std::endl;
                        return 0;
                    } else if (enc_ret < 0) {
                        if (enc_ret == AVERROR(EAGAIN))
                           break;
                        fprintf(stderr, "Error during encoding\n");
                        goto fail;
                    }
                    ePacket->stream_index = 0;
                    av_packet_rescale_ts(ePacket, ctx.ifmt_ctx->streams[videoStream]->time_base,
                                        ctx.ofmt_ctx->streams[0]->time_base);
                    if (write_flag) {
                        enc_ret = av_interleaved_write_frame(ctx.ofmt_ctx, ePacket);
                        if (enc_ret < 0) {
                            av_packet_unref(ePacket);
                            fprintf(stderr, "Error during writing data to output file. "
                                    "Error code: %d\n", enc_ret);
                            goto fail;
                        }
                    }
                    // av_packet_unref(ePacket);
                }
                step_diff = av_gettime_relative() - step_start_time_ts;
                if (step_diff * 1000.0/ AV_TIME_BASE > durationTime) {
                    printPerfStep();
                    stepTranscodeFrameNum = 0;
                    step_start_time_ts = av_gettime_relative();
                }
            }
            if (pFrame) av_frame_free(&pFrame);
        }
        // av_packet_unref(dPacket);
    }

fail:
    if (pFrame) av_frame_free(&pFrame);
    return 0;
}

std::string VideoTranscoder::getOutputName() {
    string threadstr = "_thread_";
    string suffix = ".h264";
    return threadstr + std::to_string(threadId) + suffix;
}

void VideoTranscoder::printPerf() 
{
    if (!transcode_time)
        return;
    transcodeFps =  (transcodeFrameNum  * 1.f / transcode_time * AV_TIME_BASE);
    std::cout << "[thread-fps]: -->thread id: " << threadId
              << " -->thread fps: " << std::fixed <<setprecision(3) << transcodeFps
              << " -->total frame: " << transcodeFrameNum << std::endl;
}

void VideoTranscoder::closeTranscoder() {
    // av_frame_free(&pFrame);
    if (pFrame) av_frame_free(&pFrame);
    if (ePacket) av_packet_free(&ePacket);
    if (dPacket) av_packet_free(&dPacket);
    avformat_close_input(&ctx.ifmt_ctx);
    avformat_close_input(&ctx.ofmt_ctx);
    avformat_free_context(ctx.ifmt_ctx);
    avformat_free_context(ctx.ofmt_ctx);
    avcodec_free_context(&ctx.decoder_ctx);
    avcodec_free_context(&ctx.encoder_ctx);
    avcodec_close(ctx.decoder_ctx);
    avcodec_close(ctx.encoder_ctx);
    
    //decodeContext = {};
    //encodeContext = {};
    printPerf();
    //std::cout << "close transer." << sVideotd::endl;
}

void VideoTranscoder::printPerfStep() {
    double stepFps = 0.0f;
    if (step_diff) {
        stepFps =  (stepTranscodeFrameNum * 1.f / step_diff * AV_TIME_BASE);
    }
    cout << "[stepFpsStats]: -->thread id: " << threadId <<" -->fps: "
         << std::fixed <<setprecision(2)<< stepFps << " -->step time: " << step_diff * 1000.0 / AV_TIME_BASE;
         cout.unsetf( ios::fixed);
    cout << " -->step frame count: " << stepTranscodeFrameNum;
    cout << " -->total frame count: " << transcodeFrameNum;
    cout << std::endl;
}

double VideoTranscoder::getActualFps() {
    if (transcodeFps < 0) {
        transcodeFps = 0.0f;
    }
    return transcodeFps;
}
