#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "hwdecoder.h"

#define BLANK_CEILING 5

std::vector<double> fps_stat;
std::mutex g_fps_mutex;
int analysis_txt_file(
    std::string txt_inputfile, std::vector<std::string> &inputfile,
    std::vector<std::string> &dev_id, std::vector<std::string> &save_flag) {
  std::string line_str;
  int line_num = 0;
  int empty_count = 0;
  std::ifstream mlu_para_file;
  mlu_para_file.open(txt_inputfile.c_str(), std::ios::in);

  if (mlu_para_file.is_open()) {
    while (std::getline(mlu_para_file, line_str)) {
      std::istringstream iss(line_str);
      std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{}};
      if (tokens.size() == 0) {
        empty_count++;
        if (empty_count >= BLANK_CEILING) {
          printf("%d blank lines detected, maybe it's an empty file,\
              please check your text file \n", empty_count);
          break;
        }
        continue;
      } else if (tokens[0].rfind("thread", 0) == 0) {
        inputfile.push_back(tokens[1].data());;
        dev_id.push_back(tokens[2].data());
        save_flag.push_back(tokens[3].data());
        line_num++;
      } else if (tokens[0].rfind("#", 0) == 0) {
        continue;
      } else {
        printf("Unidentified keyword in config file:%s\n", tokens[0].c_str());
        mlu_para_file.close();
        return -1;
      }
    }
  } else {
    printf("Unable to open config file\n");
    return -1;
  }
  mlu_para_file.close();
  return line_num;
}

void decoder_task(std::string filename, int dev_id,
                  int thread_id, int dump_flag) {
  int ret = -1;
  Decoder d;
  ret = d.decode_process(filename, dev_id, thread_id, dump_flag);
  if (ret) {
    printf("decode process failed\n");
    abort();
  }
  d.print_perf_total();

  std::lock_guard<std::mutex> lock(g_fps_mutex);
  fps_stat.push_back(d.get_actual_fps());
}

int main(int argc, char** argv) {
    if (argc <= 1) {
      printf("Usage:./%s <input_file> \n", argv[0]);
      return -1;
    }

    std::vector<std::thread> vec;
    std::vector<std::string> inputfile;
    std::vector<std::string> dev_id;
    std::vector<std::string> save_flag;

    std::string txt_inputfile = argv[1];
    int thread_num;

    thread_num = analysis_txt_file(txt_inputfile, inputfile, dev_id, save_flag);
    if (thread_num <= 0) {
      printf("open config file fail, please check your input file.\n");
      return -1;
    }

    for (int i = 0; i < thread_num; i++) {
      vec.push_back(std::thread(decoder_task,
                              inputfile[i],
                              atoi(const_cast<char *>(dev_id[i].c_str())),
                              i,
                              atoi(const_cast<char *>(save_flag[i].c_str()))
                              ));
    }

    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
      iter->join();
    }
    vec.clear();

    double total_avg_fps = 0.0f;
    for (auto iter = fps_stat.begin(); iter != fps_stat.end(); ++iter){
      total_avg_fps += *iter;
    }
    fps_stat.clear();

    printf("\n[TotalFpsStats] total fps:%.3f, total chan:%d, average fps:%.3f\n\n",
      total_avg_fps, thread_num, total_avg_fps/(thread_num * 1.f));

  return 0;
}
