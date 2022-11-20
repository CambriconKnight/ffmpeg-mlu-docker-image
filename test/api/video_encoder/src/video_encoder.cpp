#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <vector>
#include "video_encoder.h"
#include "time.h"

int VideoEncoder::
    init_encoder(enc_params_t enc_params, int thread_id) {
    int ret;
    int width;
    int height;
    AVDictionary *e_dict = NULL;
    std::string out_format;
    std::string source;
    std::string enc_type;
    std::string bit_rate;
    std::string b_frame;
    std::string gop_size;
    std::string rc_mode;
    std::string profile;
    std::string level;
    std::string quality;
    std::string q_min;
    std::string q_max;

    chan_id        = thread_id;
    enc_frame_num  = 0;
    duration_time  = 3000.0;
    jpeg_enc_type  = false;

    source    = enc_params.inputfile;
    enc_type  = enc_params.enc_type;
    bit_rate  = enc_params.bit_rate;
    b_frame   = enc_params.b_frame;
    gop_size  = enc_params.gop_size;
    rc_mode   = enc_params.rc_mode;
    profile   = enc_params.profile;
    level     = enc_params.level;
    quality   = enc_params.quality;
    q_min     = enc_params.q_min;
    q_max     = enc_params.q_max;

    width     = std::atoi(enc_params.enc_w.c_str());
    height    = std::atoi(enc_params.enc_h.c_str());
    dev_id    = std::atoi(enc_params.dev_id.c_str());
    write_flag     = std::atoi(enc_params.save_flag.c_str());
    need_enc_count = std::atoi(enc_params.enc_num.c_str());

    av_dict_set_int(&e_dict, "device_id", dev_id, 0);

    if (av_stristr(enc_type.c_str(), "jpeg")) {
        av_dict_set(&e_dict, "quality", quality.c_str(),  0);

        printf("[chan:%d] type:%s, devid:%d, q:%s\n",
            thread_id, enc_type.c_str(), dev_id, quality.c_str());
    } else {
        av_dict_set(&e_dict, "b",       bit_rate.c_str(), 0);
        av_dict_set(&e_dict, "bf",      b_frame.c_str(),  0);
        av_dict_set(&e_dict, "rc",      rc_mode.c_str(),  0);
        av_dict_set(&e_dict, "g",       gop_size.c_str(), 0);
        av_dict_set(&e_dict, "profile", profile.c_str(),  0);
        av_dict_set(&e_dict, "level",   level.c_str(),    0);
        av_dict_set(&e_dict, "qmin",    q_min.c_str(),    0);
        av_dict_set(&e_dict, "qmax",    q_max.c_str(),    0);

        printf("[chan:%d] type:%s, devid:%d, b:%s, bf:%s, rc:%s, g:%s, p:%s, l:%s, qmin:%s, qmax:%s\n",
            thread_id, enc_type.c_str(), dev_id, bit_rate.c_str(), b_frame.c_str(), rc_mode.c_str(),
            gop_size.c_str(), profile.c_str(), level.c_str(), q_min.c_str(), q_max.c_str());
    }
    if (av_stristr(enc_type.c_str(), "264")) {
        out_format    = "h264";
        enc_ctx.codec = avcodec_find_encoder_by_name("h264_mluenc");
    } else if (av_stristr(enc_type.c_str(), "265")
        || av_stristr(enc_type.c_str(), "evc")) {
        out_format    = "hevc";
        enc_ctx.codec = avcodec_find_encoder_by_name("hevc_mluenc");
    } else if (av_stristr(enc_type.c_str(), "jpeg")) {
        jpeg_enc_type = true;
        out_format    = "singlejpeg";
        enc_ctx.codec = avcodec_find_encoder_by_name("mjpeg_mluenc");
    } else {
        printf("error encode_type params\n");
        return -1;
    }
    if(!enc_ctx.codec) {
        printf("encoder find failed.\n");
        return -1;
    }
    if ((ret = avformat_network_init()) != 0) {
        printf("avformat network init failed, ret:%d\n", ret);
        return ret;
    }
    ret = avformat_alloc_output_context2(&enc_ctx.formatContext,
            NULL, out_format.c_str(), NULL);
    if (ret) {
        printf("avformat_alloc_output_context2 failed\n");
        return ret;
    }
    enc_ctx.videoStream = avformat_new_stream(enc_ctx.formatContext, enc_ctx.codec);
    if (!enc_ctx.videoStream) {
        printf("avformat_new_stream failed\n");
        return -1;
    }

    enc_ctx.codecContext = avcodec_alloc_context3(enc_ctx.codec);
    if (!enc_ctx.codecContext) {
        printf("could not allocate video codec context\n");
        return -1;
    }

    enc_ctx.codecContext->width  = width;
    enc_ctx.codecContext->height = height;
    enc_ctx.codecContext->time_base = (AVRational) {1, 25};
    enc_ctx.codecContext->framerate = (AVRational) {25, 1};
    enc_ctx.codecContext->pix_fmt = AV_PIX_FMT_NV12;

    ret = avcodec_parameters_from_context(
            enc_ctx.videoStream->codecpar, enc_ctx.codecContext);
    if (ret) {
        printf("avcodec_parameters_from_context failed\n");
        return ret;
    }

    ret = avcodec_open2(enc_ctx.codecContext, enc_ctx.codec, &e_dict);
    if(ret < 0) {
        printf("avcodec open failed.\n");
        return ret;
    }
    av_dict_free(&e_dict);

    e_pkt = av_packet_alloc();
    av_init_packet(e_pkt);

    p_frame         = av_frame_alloc();
    p_frame->format = AV_PIX_FMT_NV12;
    p_frame->width  = enc_ctx.codecContext->width;
    p_frame->height = enc_ctx.codecContext->height;
    ret = av_frame_get_buffer(p_frame, 32);
    if (ret) {
        printf("av_frame_get_buffer failed\n");
        return ret;
    }

    input_file = fopen(source.c_str(), "rb");
    if (!input_file) {
        printf("open inputfile:%s failed\n", source.c_str());
        return -1;
    }
    if (write_flag && !jpeg_enc_type) {
        std::string stream_name = "chan_" +
                            std::to_string(chan_id) + "." + out_format;
        output_file = fopen(stream_name.c_str(), "wb");
        if (!output_file) {
            printf("open outputfile:%s failed\n", stream_name.c_str());
            return -1;
        }
    }

    step_start_time = std::chrono::steady_clock::now();

    return 0;
}

int VideoEncoder::do_encode() {
    int ret = 0;
    int load_len = 0;
    uint8_t* picture_buf = NULL;
    uint32_t frame_size = 0;
    uint32_t plane_size = 0;
    frame_size = p_frame->width * p_frame->height * 3 / 2;
    plane_size = p_frame->width * p_frame->height;

    picture_buf = (uint8_t*)av_malloc(frame_size);
    load_len = fread(picture_buf, 1, frame_size, input_file);
    if (!load_len) {
        printf("load input file failed\n");
        return -1;
    } else if ((uint32_t)load_len != (frame_size)) {
        printf("invalid input size \n");
        return -1;
    }
    if (p_frame) {
        memcpy(p_frame->data[0], picture_buf, plane_size);
        memcpy(p_frame->data[1], picture_buf + plane_size, plane_size / 2);
        free(picture_buf);
        picture_buf = NULL;
    } else {
        free(picture_buf);
        picture_buf = NULL;
        printf("error p_frame is NULL\n");
        return -1;
    }
    while (need_enc_count--) {
        p_frame->pts     = enc_frame_num++;
        ret = avcodec_send_frame(enc_ctx.codecContext, p_frame);
        if (ret < 0 && ret != AVERROR_EOF) {
            if (ret == AVERROR(EAGAIN)) continue;
            printf("sending a frame to encode failed\n");
            return -1;
        }
        while(ret >= 0) {
            ret = avcodec_receive_packet(enc_ctx.codecContext, e_pkt);
            if (ret == AVERROR(EAGAIN)) {
              continue;
            } else if (ret == AVERROR_EOF) {
                break;
            } else if (ret < 0) {
                printf("error during encoding \n");
                return -1;
            }

            print_perf_step();
            if (write_flag) {
                if (!jpeg_enc_type) {
                    fwrite(e_pkt->data, 1, e_pkt->size, output_file);
                } else {
                    if(img_write(e_pkt, enc_frame_num)) {
                        printf("write image failed\n");
                        return -1;
                    }
                }
            }
            av_packet_unref(e_pkt);
        }
        av_packet_unref(e_pkt);
    }

    ret = avcodec_send_frame(enc_ctx.codecContext, NULL);
    if (ret < 0) {
        printf("error sending eof frame to encoder \n");
        return -1;
    }
    while(ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx.codecContext, e_pkt);
        if (ret == AVERROR_EOF) {
            break;
        } else if (ret == AVERROR(EAGAIN)) {
            continue;
        } else if (ret < 0) {
            printf("error during encoding \n");
            return -1;
        }
        if (write_flag) {
            if (!jpeg_enc_type) {
                fwrite(e_pkt->data, 1, e_pkt->size, output_file);
            } else {
                if(img_write(e_pkt, enc_frame_num)) {
                    printf("write image failed\n");
                    return -1;
                }
            }
        }
        av_packet_unref(e_pkt);
    }

    return 0;
}

void VideoEncoder::close_encoder() {
    if (input_file)  fclose(input_file);
    if (write_flag && !jpeg_enc_type) {
        if (output_file) fclose(output_file);
    }
    if (p_frame) av_frame_free(&p_frame);
    if (e_pkt)   av_packet_free(&e_pkt);
    avformat_close_input(&enc_ctx.formatContext);
    avcodec_free_context(&enc_ctx.codecContext);
}

void VideoEncoder::print_perf_step() {
    double step_fps = 0.0f;
    step_enc_num ++;
    step_end_time = std::chrono::steady_clock::now();
    step_diff   = step_end_time - step_start_time;
    if (step_diff.count() > duration_time) {
        step_fps =  (step_enc_num * 1000 * 1.f / step_diff.count());
        printf("[stepFps] chan:%03d, fps:%.02f, step time:%.02f, step count:%d, frame count:%d\n",
            chan_id, step_fps, step_diff.count(), step_enc_num, enc_frame_num);
        step_enc_num = 0;
        step_start_time = std::chrono::steady_clock::now();
    }
}

int VideoEncoder::img_write(AVPacket *e_pkt, int img_idx) {
    int ret = -1;
    std::string img_name = "";
    img_name = "img_chan" + std::to_string(chan_id) +
                "_" + std::to_string(img_idx) + ".jpeg";

    if (avio_open(&enc_ctx.formatContext->pb, img_name.c_str(), AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file\n");
        return -1;
    }
    ret = avformat_write_header(enc_ctx.formatContext, NULL);
    if(ret < 0) {
        printf("avformat_write_header failed\n");
        return ret;
    }
    ret = av_write_frame(enc_ctx.formatContext, e_pkt);
    if(ret < 0) {
        printf("av_write_frame failed \n");
        return ret;
    }
    ret = av_write_trailer(enc_ctx.formatContext);
    if(ret < 0) {
        printf("av_write_trailer failed \n");
        return -1;
    }
    avio_close(enc_ctx.formatContext->pb);

    printf("%s\n", img_name.c_str());
    return 0;
}
