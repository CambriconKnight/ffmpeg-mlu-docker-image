#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <thread>
#include <vector>
#include<iomanip>
#include <unordered_map>
#include "transcode.h"

#define BLANK_CEILING 5

std::vector<double> fps_stat;
 int analysis_txt_file(std::string txt_inputfile,
                       std::vector<std::string> &inputfile,
                       std::vector<std::string> &dst_w,
                       std::vector<std::string> &dst_h,
                       std::vector<std::string> &filters_desc,
                       std::vector<std::string> &dev_id,
                       std::vector<std::string> &save_flag) {
    std::string line_str;
    int line_num = 0;
    int empty_count = 0;
    std::ifstream mlu_para_file;
    mlu_para_file.open(txt_inputfile.c_str(), std::ios::in);

    if (mlu_para_file.is_open()) {
        while (std::getline(mlu_para_file, line_str)) {
            std::istringstream iss(line_str);
            std::vector<std::string> tokens{
                        std::istream_iterator<std::string>{iss},
                        std::istream_iterator<std::string>{} };
            if (tokens.size() == 0) {
                //std::cout << "line is empty." << std::endl;
                empty_count++;
                if (empty_count >= BLANK_CEILING) {
                    std::cout << empty_count
                              << " blank lines detected, "
                              << "maybe it's an empty file, "
                              << "please check your text file."
                              << std::endl;
                    break;
                }
                continue;
            } else if (tokens[0].rfind("thread", 0) == 0) {
                inputfile.push_back(tokens[1].data());
                dst_w.push_back(tokens[2].data());
                dst_h.push_back(tokens[3].data());
                dev_id.push_back(tokens[4].data());
                save_flag.push_back(tokens[5].data());

                std::string filter_desc;
                if (tokens[6] == "mlu") {
                    filter_desc = "scale_yuv2yuv_mlu=" + dst_w[line_num]
                                + ":" + dst_h[line_num] + ":" + dev_id[line_num];
                } else {
                    filter_desc = "scale=" + dst_w[line_num] + ":" + dst_h[line_num];
                }

                filters_desc.push_back(filter_desc);
                (line_num)++;
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
    return line_num;
}

void infer_task(std::string input_file, std::string filter_desc,
                int dst_w, int dst_h, int dev_id, int thread_id, int save_flag) {
    Transcode f;
    std::chrono::time_point<std::chrono::steady_clock> transcode_start_time;
    std::chrono::time_point<std::chrono::steady_clock> transcode_end_time;
    std::chrono::duration<double, std::milli> transcode_time;

    transcode_start_time = transcode_end_time = std::chrono::steady_clock::now();

	f.filter(input_file, filter_desc, dst_w, dst_h, dev_id, thread_id, save_flag);
    transcode_end_time = std::chrono::steady_clock::now();
    transcode_time = transcode_end_time - transcode_start_time;
    f.thread_fps_ = f.infer_frame_num_ * 1000.0 * 1.f / transcode_time.count();
    printf("f.thread[%d]_fps = %f\n", thread_id, f.thread_fps_);
    std::cout.unsetf(std::ios::fixed);
    fps_stat.push_back(f.thread_fps_);
}

int main(int argc, char** argv) {
    if(argc <= 1) {
        std::cout << "Usage: "<< argv[0] <<" <input_file>" << std::endl;
        return -1;
    }
    std::vector<std::thread> vec;
    std::vector<std::string> inputfile;
    std::vector<std::string> dst_w;
    std::vector<std::string> dst_h;
    std::vector<std::string> filters_desc;
    std::vector<std::string> dev_id;
    std::vector<std::string> save_flag;

    std::string txt_inputfile = argv[1];
    int thread_num;

    thread_num = analysis_txt_file(txt_inputfile, inputfile,
                                  dst_w, dst_h, filters_desc, dev_id, save_flag);
    std::cout << "---------------------------------------------------------------------" << std::endl;
    std::cout << "[txt_inputfile]: --> " << txt_inputfile << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;

    if(thread_num <= 0) {
        std::cout << "open config file fail, please check your input file." << std::endl;
        return -1;
    }
    for(int i = 0; i < thread_num; i++) {
        vec.push_back(std::thread(infer_task,
                                    inputfile[i],
                                    filters_desc[i],
                                    atoi(const_cast<char *>(dst_w[i].c_str())),
                                    atoi(const_cast<char *>(dst_h[i].c_str())),
                                    atoi(const_cast<char *>(dev_id[i].c_str())),
                                    i,
                                    atoi(const_cast<char *>(save_flag[i].c_str()))
                                ));
    }
    for(auto iter = vec.begin(); iter != vec.end(); ++iter) {
        if(iter->joinable()) {
            iter->join();
        }
    }
    double total_avg_fps = 0.0f;
    for(auto iter = fps_stat.begin(); iter != fps_stat.end(); ++iter) {
        total_avg_fps += *iter;
    }
    std::cout << std::endl;
    std::cout << "---------------------------------------------------------------------" << std::endl;
    std::cout << "[FpsStats]: --> " << std::fixed << std::setprecision(2) \
              << "TotalFps: " << total_avg_fps << " --> Average Fps: " \
              << total_avg_fps / (thread_num * 1.f) << std::endl;
              std::cout.unsetf(std::ios::fixed);
    std::cout << "---------------------------------------------------------------------" << std::endl;
    return 0;
}
