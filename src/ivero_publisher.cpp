#include <chrono>
#include <memory>
#include <pcl_thread.h>
#include <rgb_thread.h>
#include <string>
#include <thread>
using namespace std::chrono_literals;

void RGB_image_thread(rs2::pipeline &pipe) {
  auto RGB_node = std::make_shared<RgbImage>(pipe);
  RGB_node->Start();
}
void PCL_thread(rs2::pipeline &pipe) {
  auto PCL_node = std::make_shared<PCLStream>(pipe);
  PCL_node->Start();
}

int main(int argc, char *argv[]) {

  // Create a RealSense pipeline
  rs2::pipeline pipeline;
  rs2::config config;
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH, 848, 480, RS2_FORMAT_ANY,
                       30);

  config.enable_stream(rs2_stream::RS2_STREAM_COLOR, 848, 480, RS2_FORMAT_BGR8,
                       30);
  pipeline.start(config);

  std::thread RGB(RGB_image_thread, std::ref(pipeline));
  std::thread PCL(PCL_thread, std::ref(pipeline));
  PCL.join();
  RGB.join();

  return 0;
}
