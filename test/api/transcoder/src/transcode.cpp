#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include "transcode.h"
#include "time.h"
#include "assert.h"

#define PRINT_PERF 1
#define STRIDE_ALIGN 1

void Transcode::filter(std::string inputfile, std::string filter_desc,
                       int dst_w, int dst_h, int device_id, int thread_id, int save_flag) {
    if (device_id >= 0) {
        dev_id_ = device_id;
    }
    write_flag_ = save_flag;
    thread_id_ = thread_id;
    duration_time_ = 3000.0;

    int ret;
    if ((ret = init_decoder(inputfile, dev_id_)) < 0) {
        printf("init decoder fail:%d\n", ret);
        goto failed;
    }
    if ((ret = init_encoder(get_output_name(), dst_w, dst_h, write_flag_)) < 0) {
        printf("init encoder fail:%d\n", ret);
        goto failed;
    }
    if ((ret = init_filter(filter_desc)) < 0) {
        printf("init filter fail:%d\n", ret);
        goto failed;
    }
    if ((ret = filtering()) < 0) {
        printf("filtering fail:%d\n", ret);
        goto failed;
    }
failed:
    close_filter();
}

int Transcode::init_filter(std::string filter_desc) {
    int ret;
    char args[512];
    const AVFilter *buffersrc  = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");
    outputs = avfilter_inout_alloc();
    inputs  = avfilter_inout_alloc();
    enum AVPixelFormat pix_fmts[] = {AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE};
    AVBufferSinkParams *buffersink_params;
    filter_graph = avfilter_graph_alloc();
    //nb_threads设置解码器线程数,默认是不需要设置的,即自适应,可能会消耗一些线程数量.
    filter_graph->nb_threads = 1;
    //buffer video source
    snprintf(args, sizeof(args),
             "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
             decoder_ctx->width, decoder_ctx->height,
             decoder_ctx->pix_fmt,
             decoder_ctx->time_base.num,
             decoder_ctx->time_base.den,
             decoder_ctx->sample_aspect_ratio.num,
             decoder_ctx->sample_aspect_ratio.den);
    ret = avfilter_graph_create_filter(&buffersrc_ctx,
                                       buffersrc,
                                       "in",
                                       args,
                                       NULL,
                                       filter_graph);
    if (ret < 0) {
        printf("Cannot create buffer source\n");
    }
    //buffer video sink
    buffersink_params = av_buffersink_params_alloc();
    buffersink_params->pixel_fmts = pix_fmts;
    ret = avfilter_graph_create_filter(&buffersink_ctx,
                                       buffersink,
                                       "out",
                                       NULL,
                                       buffersink_params,
                                       filter_graph);
    av_free(buffersink_params);
    if (ret < 0) {
        printf("Cannot create buffersink\n");
        return ret;
    }
    /* Endpoints for the filter graph. */
    outputs->name       = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx    = 0;
    outputs->next       = NULL;

    inputs->name       = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx    = 0;
    inputs->next       = NULL;
    if ((ret = avfilter_graph_parse_ptr(filter_graph,
                                        filter_desc.c_str(),
                                        &inputs, &outputs, NULL)) < 0) {
        printf("parse diy filter fail:%d\n", ret);
        return ret;
    }
    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0) {
        printf("config diy filter fail :%d\n", ret);
        return ret;
    }
    return 0;
}

int Transcode::init_decoder(std::string source, int device_id) {
    int ret;
    AVCodec *dec;
    AVStream *in_stream;
    AVDictionary *options = NULL;
    dev_id_ = device_id;
    if ((ret = avformat_network_init()) != 0) {
        printf("avformat_network_init failed, ret: %d\n",ret);
        return ret;
    }
    if (!strncmp(source.c_str(), "rtsp", 4) || !strncmp(source.c_str(), "rtmp", 4)) {
        printf("decode rtsp/rtmp stream \n");
        av_dict_set(&options, "buffer_size",    "1024000", 0);
        av_dict_set(&options, "max_delay",      "500000", 0);
        av_dict_set(&options, "stimeout",       "20000000", 0);
        av_dict_set(&options, "rtsp_transport", "tcp", 0);
    } else {
        printf("decode local file stream \n");
        av_dict_set(&options, "stimeout", "20000000", 0);
        av_dict_set(&options, "vsync", "0", 0);
    }

    ifmt_ctx = avformat_alloc_context();
    ret = avformat_open_input(&ifmt_ctx, source.c_str(), NULL, &options);
    if (ret != 0) {
        av_dict_free(&options);
        printf("avformat_open_input failed, ret:%d\n", ret);
        return ret;
    }
    av_dict_free(&options);
    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0) {
        printf("Cannot find stream information\n");
        return ret;
    }
    /* select the video stream */
    ret = av_find_best_stream(ifmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (ret < 0) {
        printf("Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream = ret;
    in_stream = ifmt_ctx->streams[video_stream];
    switch(in_stream->codecpar->codec_id) {
        case AV_CODEC_ID_H264:
            dec = avcodec_find_decoder_by_name("h264_mludec");
            break;
        case AV_CODEC_ID_HEVC:
            dec = avcodec_find_decoder_by_name("hevc_mludec");
            break;
        case AV_CODEC_ID_VP8:
            dec = avcodec_find_decoder_by_name("vp8_mludec");
            break;
        case AV_CODEC_ID_VP9:
            dec = avcodec_find_decoder_by_name("vp9_mludec");
            break;
        case AV_CODEC_ID_MJPEG:
            dec = avcodec_find_decoder_by_name("mjpeg_mludec");
            break;
        default:
            dec = avcodec_find_decoder(in_stream->codecpar->codec_id);
            break;
    }
    if( !dec ) {
        printf("decoder find failed.\n");
        return AVERROR(ENODEV);
    }
    if (!(decoder_ctx = avcodec_alloc_context3(dec))){
        printf("alloc decoder context fail\n");
        return AVERROR(ENOMEM);
    }
    if ((ret = avcodec_parameters_to_context(decoder_ctx, in_stream->codecpar)) < 0) {
        printf("avcodec_parameters_to_context error. Error code:%d\n", ret);
        return ret;
    }
    if (av_stristr(dec->name,"mludec")) {
        av_dict_set_int(&d_avdict, "device_id", dev_id_, 0);
        av_dict_set_int(&d_avdict, "stride_align", STRIDE_ALIGN, 0);
        // av_dict_set(&d_avdict, "resize", "960x540", 0);
    }
    //thread_count设置解码器线程数,默认是不需要设置的,即自适应,可能会消耗一些线程数量.
    decoder_ctx->thread_count = 1;
    if ((ret = avcodec_open2(decoder_ctx, dec, &d_avdict)) < 0) {
        printf("Failed to open codec for decoding, Error code: %d\n", ret);
        return ret;
    }
    decoder_ctx->time_base.num = 1;
    decoder_ctx->time_base.den = 25;
    if (d_avdict) {
        av_dict_free(&d_avdict);
        d_avdict = NULL;
    }

    return ret;
}

int Transcode::init_encoder(std::string outputfile,
                            int dst_w, int dst_h, int write_flag_) {
    int ret = 0;
    AVCodec *enc;
    AVStream *out_stream;
    if ((ret = (avformat_alloc_output_context2(&(ofmt_ctx), NULL, NULL, outputfile.c_str()))) < 0) {
        printf("Failed to deduce output format from file extension, Error code:%d\n", ret);
        return ret;
    }
    enc = avcodec_find_encoder_by_name("h264_mluenc");
    if( !enc ) {
        printf("encoder find failed.\n");
        return AVERROR(ENODEV);
    }
    //printf("find encoder successfully\n");
    if (!(encoder_ctx = avcodec_alloc_context3(enc))) {
        return AVERROR(ENOMEM);
    }
    if (write_flag_) {
        ret = avio_open(&(ofmt_ctx->pb), outputfile.c_str(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf("Cannot open output file. Error code: %d\n", ret);
            return ret;
        }
    }
    encoder_ctx->time_base = (AVRational){1, 25};
    /* take first format from list of supported formats */
    if (enc->pix_fmts && enc->pix_fmts[0] != 0) {
        encoder_ctx->pix_fmt = enc->pix_fmts[0];
    }
    else {
        encoder_ctx->pix_fmt = AV_PIX_FMT_NV12;
    }
    encoder_ctx->width     = dst_w;
    encoder_ctx->height    = dst_h;
    encoder_ctx->bit_rate = DEFAULT_BPS;
    encoder_ctx->gop_size = DEFAULT_GOP;
    encoder_ctx->sample_aspect_ratio = decoder_ctx->sample_aspect_ratio;
    encoder_ctx->framerate =  (AVRational){25, 1};
    encoder_ctx->qmin = DEFAULT_QMIN;
    encoder_ctx->qmax = DEFAULT_QMAX;
    if (av_stristr(enc->name,"mluenc")) {
        av_dict_set_int(&e_avdict, "device_id", dev_id_, 0);
        av_dict_set_int(&d_avdict, "stride_align", STRIDE_ALIGN, 0);
    }
    //thread_count设置解码器线程数,默认是不需要设置的,即自适应,可能会消耗一些线程数量.
    encoder_ctx->thread_count = 1;
    if ((ret = avcodec_open2(encoder_ctx, enc, &e_avdict)) < 0) {
        fprintf(stderr, "Failed to open encode codec. "
                        "Error code: %d\n", ret );
        return ret;
    }
    if (e_avdict) {
        av_dict_free(&e_avdict);
        e_avdict = NULL;
    }
    if (!(out_stream = avformat_new_stream(ofmt_ctx, enc))) {
        fprintf(stderr, "Failed to allocate stream for output format.\n");
        return AVERROR(ENOMEM);
    }
    out_stream->time_base = encoder_ctx->time_base;
    ret = avcodec_parameters_from_context(out_stream->codecpar, encoder_ctx);
    if (ret < 0) {
        fprintf(stderr, "Failed to copy the stream parameters. "
                "Error code: %d\n", ret);
        return ret;
    }
    if (write_flag_) {
        if ((ret = avformat_write_header(ofmt_ctx, NULL)) < 0) {
            fprintf(stderr, "Error while writing stream header. "
                    "Error code: %d\n", ret);
            return ret;
        }
    }

    return ret;
}

int Transcode::encode_video_frame(AVFrame *filter_out_frame) {
    int enc_ret = 0;
    e_packet = av_packet_alloc();
    if ( !e_packet ) {
        av_log(NULL, AV_LOG_ERROR,
               "enc alloc pkt failed, func: %s, line: %d\n", __func__, __LINE__);
        return AVERROR(ENOMEM);
    }
    av_init_packet(e_packet);

    enc_ret = avcodec_send_frame(encoder_ctx, filter_out_frame);
    if (enc_ret < 0 && enc_ret != AVERROR_EOF) {
        av_log(NULL, AV_LOG_ERROR,
               "enc send frame failed, ret(%d), func: %s, line: %d\n", enc_ret, __func__, __LINE__);
        av_packet_free(&e_packet);
        return enc_ret;
    }
    while (enc_ret >= 0) {
        enc_ret = avcodec_receive_packet(encoder_ctx, e_packet);
        if (enc_ret < 0 && enc_ret != AVERROR_EOF) {
            // av_log(NULL, AV_LOG_ERROR,
            //     "enc receice pkt failed, ret(%d), func: %s, line: %d\n", enc_ret, __func__, __LINE__);
            if (enc_ret == AVERROR(EAGAIN)) { // debug all return EAGAIN
                enc_ret = 0;
            }
            av_packet_free(&e_packet);
            break;
        } else if (enc_ret == AVERROR_EOF && write_flag_) {
            av_write_trailer(ofmt_ctx);
            av_packet_free(&e_packet);
            return 0;
        }

        if (write_flag_) {
            e_packet->stream_index = 0;
            av_packet_rescale_ts(e_packet,
                                ifmt_ctx->streams[video_stream]->time_base,
                                ofmt_ctx->streams[0]->time_base);
            enc_ret = av_interleaved_write_frame(ofmt_ctx, e_packet);
            if (enc_ret < 0) {
                printf("Error during writing data to output file, error code: %d\n", enc_ret);
            }
        }
        av_packet_unref(e_packet);
    }
    av_packet_free(&e_packet);
    return enc_ret;
}

int Transcode::filter_write_video_frame(AVFrame *filter_in_frame) {
    int ret;
    int write_trailer_flag = 0;

    ret = av_buffersrc_add_frame_flags(buffersrc_ctx, filter_in_frame, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR,
               "Error while feeding the filtergraph, ret(%d)\n", ret);
        return ret;
    }
    // pull filterred frames from filtergraphs
    while (1) {
        AVFrame *filted_frame = av_frame_alloc();
        if (!filted_frame) {
            av_log(NULL, AV_LOG_ERROR,
                  "Error while alloc frame for filtergraph \n");
            return AVERROR(ENOMEM);
        }
        ret = av_buffersink_get_frame(buffersink_ctx, filted_frame);
        if (ret < 0) {
            // av_log(NULL, AV_LOG_ERROR, "buf sink get frame failed, ret(%d), func: %s, line: %d \n",
            //         ret, __func__, __LINE__);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                if (ret == AVERROR_EOF) {
                    write_trailer_flag = 1;
                }
                ret = 0;
            }
            av_frame_free(&filted_frame);
            break;
        }
        ret = encode_video_frame(filted_frame);
        if (ret < 0) {
            // av_log(NULL, AV_LOG_ERROR, "encode_video_frame failed, ret(%d), func: %s, line: %d \n", ret, __func__, __LINE__);
            // av_log(NULL, AV_LOG_ERROR, "encode_video_frame failed, ret(%d) \n", ret);
        }
        av_frame_free(&filted_frame);
    }
    if (write_trailer_flag == 1) {
        encode_video_frame(NULL);
    }
    return ret;
}

int Transcode::filtering() {
    int ret;
    int dec_ret = 0;
    int is_get_eof = 0;
    d_packet = av_packet_alloc();
    av_init_packet(d_packet);
    #if PRINT_PERF
    step_start_time = std::chrono::steady_clock::now();
    #endif
    while(dec_ret >= 0 || dec_ret == AVERROR(EAGAIN)) {
        dec_ret = av_read_frame(ifmt_ctx, d_packet);
        if (dec_ret < 0) {
            if (dec_ret == AVERROR_EOF) {
                is_get_eof = 1;
                fprintf(stderr, "av_read_frame get eof \n");
            } else {
                if (d_packet) {
                    av_packet_free(&d_packet);
                }
                fprintf(stderr, "av_read_frame failed, ret: %d\n", dec_ret);
                return AVERROR(ENOMEM);
            }
        }
        if (is_get_eof) {
            d_packet->data = NULL;
            d_packet->size = 0;
        }
        if (video_stream != d_packet->stream_index) {
            // fprintf(stderr, "video_stream(%d) != d_packet->stream_index(%d)\n", video_stream, d_packet->stream_index);
            av_packet_unref(d_packet);
            continue;
        }
        //stream_index = d_packet->stream_index;
        dec_ret = avcodec_send_packet(decoder_ctx, d_packet);
        if (dec_ret < 0 && dec_ret != AVERROR_EOF) {
            if (d_packet) {
                av_packet_free(&d_packet);
            }
            fprintf(stderr, "Error during decoding. Error code: %d\n", dec_ret);
            return dec_ret;
        }
        while ( dec_ret >= 0) {
            if (!(p_frame = av_frame_alloc())) {
                fprintf(stderr, "dec av_frame_alloc failed \n");
                return AVERROR(ENOMEM);
            }
            dec_ret = avcodec_receive_frame(decoder_ctx, p_frame);
            if (dec_ret < 0 && dec_ret != AVERROR_EOF) {
                // fprintf(stderr, "dec receive frame failed. %d, func: %s, line: %d \n", dec_ret, __func__, __LINE__);
                av_frame_free(&p_frame);
                break;
            } else if (dec_ret == 0 || dec_ret == AVERROR_EOF) {
                if (dec_ret == AVERROR_EOF) {
                   fprintf(stderr, "dec receive frame get eof. %d, func: %s, line: %d \n", dec_ret, __func__, __LINE__);
                   p_frame = NULL;
                }

                ret = filter_write_video_frame(p_frame);
                if (ret < 0) {
                    printf("something wrong "
                                 "during filtering and encoding\n");
                }
                infer_frame_num_++;
                #if PRINT_PERF
                step_infer_frame_num_++;
                step_end_time = std::chrono::steady_clock::now();
                step_diff = step_end_time - step_start_time;
                if (step_diff.count() > duration_time_) {
                    print_perf_step();
                    step_infer_frame_num_ = 0;
                    step_start_time = std::chrono::steady_clock::now();
                }
                #endif
                av_frame_free(&p_frame);
            }
        }
        av_packet_unref(d_packet);
    }
    return 0;
}

void Transcode::close_filter() {
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);
    avfilter_graph_free(&filter_graph);
    if (p_frame) av_frame_free(&p_frame);
    if (e_packet) av_packet_free(&e_packet);
    if (d_packet) av_packet_free(&d_packet);
    avformat_close_input(&ifmt_ctx);
    avformat_close_input(&ofmt_ctx);
    avformat_free_context(ifmt_ctx);
    avformat_free_context(ofmt_ctx);
    avcodec_free_context(&decoder_ctx);
    avcodec_free_context(&encoder_ctx);
    avcodec_close(decoder_ctx);
    avcodec_close(encoder_ctx);
}

std::string Transcode::get_output_name() {
    std::string threadstr = "transcode_";
    std::string suffix = ".mp4";
    return threadstr + std::to_string(thread_id_) + suffix;
}

void Transcode::print_perf_step() {
    double step_fps = 0.0f;
    if (step_diff.count()) {
        step_fps =  (step_infer_frame_num_ * 1000 * 1.f / step_diff.count());
    }
    printf("[stepFps]->thread id:%03d, fps:%.02f, step time:%.02f, step count:%d, frame counf:%d\n",
        thread_id_, step_fps, step_diff.count(), step_infer_frame_num_, infer_frame_num_);
}

double Transcode::get_actual_fps() {
    if (actual_fps_ < 0) {
        actual_fps_ = 0.0f;
    }
    return actual_fps_;
}
