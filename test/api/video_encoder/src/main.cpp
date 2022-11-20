#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <thread>
#include <mutex>
#include <vector>
#include <iomanip>
#include "video_encoder.h"

#define BLANK_CEILING 5
std::vector<double> fps_stat;
std::mutex g_fps_mutex;

int analysis_txt_file(std::string &txt_inputfile,
                      std::vector<enc_params_t> &enc_params) {
    int empty_count = 0;
    int thread_count = 0;
    std::string line_str;
    enc_params_t e_params = {
                "1M", "0", "50", "vbr", "main", "5.1", "80", "26","56"};

    std::ifstream mlu_para_file;
    mlu_para_file.open(txt_inputfile.c_str(), std::ios::in);
    if (mlu_para_file.is_open()) {
        while (std::getline(mlu_para_file, line_str)) {
            std::istringstream iss(line_str);
            std::vector<std::string> tokens{
                std::istream_iterator<std::string>{iss},
                std::istream_iterator<std::string>{} };
            if (tokens.size() == 0) {
                empty_count++;
                if (empty_count >= BLANK_CEILING) {
                    std::cout << empty_count << " blank lines detected, "
                              << "maybe it's an empty file, "
                              << "please check your text file." << std::endl;
                    break;
                }
                continue;
            } else if (tokens[0].rfind("thread", 0) == 0) {
                enc_params[thread_count].inputfile = tokens[1].data();
                enc_params[thread_count].enc_type  = tokens[2].data();
                enc_params[thread_count].enc_w     = tokens[3].data();
                enc_params[thread_count].enc_h     = tokens[4].data();
                enc_params[thread_count].enc_num   = tokens[5].data();
                enc_params[thread_count].dev_id    = tokens[6].data();
                enc_params[thread_count].save_flag = tokens[7].data();
                thread_count++;
            } else if (tokens[0].rfind("enc_params", 0) == 0) {
                for (auto iter = tokens.begin(); iter != tokens.end(); iter++) {
                    if (*iter == "bit_rate") {
                        e_params.bit_rate = *(iter+1);
                    } else if (*iter == "b_frame") {
                        e_params.b_frame = *(iter+1);
                    } else if (*iter == "gop_size") {
                        e_params.gop_size = *(iter+1);
                    } else if (*iter == "rc_mode") {
                        e_params.rc_mode = *(iter+1);
                    } else if (*iter == "profile") {
                        e_params.profile = *(iter+1);
                    } else if (*iter == "level") {
                        e_params.level = *(iter+1);
                    } else if (*iter == "quality") {
                        e_params.quality = *(iter+1);
                    } else if (*iter == "q_min") {
                        e_params.q_min = *(iter+1);
                    } else if (*iter == "q_max") {
                        e_params.q_max = *(iter + 1);
                    }
                }
            } else if (tokens[0].rfind("#", 0) == 0) {
                continue;
            } else {
                std::cout << "Unidentified keyword in config file "
                          << tokens[0] << std::endl;
                mlu_para_file.close();
                return -1;
            }
        }
    } else {
        std::cout << "Unable to open config file " << std::endl;
        return -1;
    }
    mlu_para_file.close();
    for (int i = 0; i < thread_count; i++) {
        enc_params[i].bit_rate = e_params.bit_rate;
        enc_params[i].b_frame  = e_params.b_frame;
        enc_params[i].gop_size = e_params.gop_size;
        enc_params[i].rc_mode  = e_params.rc_mode;
        enc_params[i].profile  = e_params.profile;
        enc_params[i].level    = e_params.level;
        enc_params[i].quality  = e_params.quality;
        enc_params[i].q_min    = e_params.q_min;
        enc_params[i].q_max    = e_params.q_max;
    }

    return thread_count;
}

void encode_task(enc_params_t enc_param, int thread_id) {
    VideoEncoder encode;

    if(encode.init_encoder(enc_param, thread_id) < 0 ) {
        printf("chan[%d] init encoder failed!\n", thread_id);
        return;
    }

    encode.enc_start_time = std::chrono::steady_clock::now();
    if(encode.do_encode() < 0) {
        printf("thread[%d] encode error, while decoding\n", thread_id);
        return;
    }

    encode.enc_end_time = std::chrono::steady_clock::now();
    encode.enc_time = encode.enc_end_time - encode.enc_start_time;

    double enc_time = encode.enc_time.count();
    double fps = (encode.enc_frame_num * 1000 * 1.f) / enc_time;

    printf("[chanStats] chan=%d, fps=%.3f, time=%.3fms, enc latency=%.3fms\n",
        thread_id, fps, enc_time, enc_time / (encode.enc_frame_num * 1.f));

    encode.close_encoder();

    std::lock_guard<std::mutex> lock(g_fps_mutex);
    fps_stat.push_back(fps);

    return;
}

int main(int argc, char** argv) {
    if (argc <= 1) {
        printf("Usage:%s input.txt\n", argv[0]);
        return -1;
    }

    std::vector<std::thread> vec_th;
    std::vector<enc_params_t>  enc_params(256);
    // enc_params_t enc_params = {
    //             "1M", "0", "50", "vbr", "main", "5.1", "80", "26","56"};
    std::string txt_inputfile = argv[1];

    int thread_num;
    thread_num = analysis_txt_file(txt_inputfile, enc_params);
    if(thread_num <= 0) {
        printf("open cfg file failed, please check your input file.\n");
        return -1;
    }

    for (int i = 0; i < thread_num; i++) {
        vec_th.emplace_back(std::thread(encode_task, enc_params[i], i));
    }
    for (auto &th : vec_th) {
        th.join();
    }
    vec_th.clear();

    double total_fps = 0.0f;
    for (auto iter = fps_stat.begin(); iter != fps_stat.end(); ++iter) {
        total_fps += *iter;
    }
    fps_stat.clear();

    printf("[TotalFpsStats] total fps:%.3f, total chan:%d, average fps:%.3f\n",
        total_fps, thread_num, total_fps / (thread_num * 1.f));

    return 0;
}
