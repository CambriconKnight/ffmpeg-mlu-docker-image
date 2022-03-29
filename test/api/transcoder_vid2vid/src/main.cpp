#include <iostream>
#include <thread>
#include <vector>
#include <iomanip>
#include <unordered_map>
#include "vid2vid_trans.h"

std::vector<double> fps_stat;
void transcode_task(std::string filepath, int dst_w, int dst_h, int deviceid, int thread_num, int write_flag) {
    VideoTranscoder transcoder;
    int ret;
    if((ret = transcoder.initTranscoder(filepath, dst_w, dst_h, deviceid, thread_num, write_flag)) < 0 ) {
        std::cout<< "thread_:"<< thread_num << " init transcoder failed! Error: " << ret << std::endl;;
        return;
    }
    if((ret = transcoder.transcode()) < 0) {
        std::cout << "thread_:"<< thread_num << " error while trascoding! Error: " << ret << std::endl;
        return;
    }
    transcoder.closeTranscoder();
    fps_stat.push_back(transcoder.getActualFps());
    return;
}

int main(int argc, char** argv) {
    if (argc < 5) {
        std::cout << "Usage: "<< argv[0] <<" <file_path> <dst_w> <dst_h> <device_id> <thread_num> <save_flag>" << std::endl;
        return -1;
    }
    char *libvar = NULL;
    putenv("CNCODEC_SELECT_DEC_VPU_FOR_TRANSCODE=1");
    libvar = getenv("CNCODEC_SELECT_DEC_VPU_FOR_TRANSCODE");
    printf("CNCODEC_SELECT_DEC_VPU_FOR_TRANSCODE is: %s \n", libvar);

    std::string filepath = argv[1];
    int dst_w = atoi(argv[2]);
    int dst_h = atoi(argv[3]);
    int device_id = atoi(argv[4]);
    int thread_num = atoi(argv[5]);
    int save_flag = atoi(argv[6]);
    std::vector<std::thread> vec_th;
    std::cout << "-------------------------" << std::endl;
    std::cout << "file_path: " << filepath << std::endl;
    std::cout << "device_id: " << device_id
              << ", dst_w: " << dst_w
              << ", dst_h: " << dst_h
              << ", thread_num: " << thread_num
              << ", save_flag: " << save_flag << std::endl;
    std::cout << "-------------------------" << std::endl;
    
    for (int i = 0; i < thread_num; i++) {
        vec_th.emplace_back(transcode_task, filepath, dst_w, dst_h, device_id, i, save_flag);
    }

    for (auto &th:vec_th) {
        if (th.joinable()) {
            th.join();
        }
    }
    double totalAvgFps = 0.0f;
    for (auto iter = fps_stat.begin(); iter != fps_stat.end(); ++iter) {
        totalAvgFps += *iter;
    }
    std::cout << "[FpsStats]: --> " << std::fixed << std::setprecision(3)
         << "Total Fps: " << totalAvgFps << " --> Average Fps: "\
         << totalAvgFps / (atoi(argv[5]) * 1.f) << std::endl;
    return 0;
}
