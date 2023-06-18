#ifndef __RGB_THREAD_H__
#define __RGB_THREAD_H__

#include <chrono>
#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <zmq.hpp>

class RgbImage {

public:
  RgbImage(rs2::pipeline &pipe) : pipeline_(pipe) {
    context_ = std::make_shared<zmq::context_t>(1);
    publisher_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_PUB);
    publisher_->bind("tcp://*:5555");
  }
  ~RgbImage() {
    pipeline_.stop();
    context_->close();
    publisher_->close();
  }
  void Start() {
    while (true) {
      rs2::frameset frames = pipeline_.wait_for_frames();
      rs2::frame color_frame = frames.get_color_frame();
      cv::Mat frame(cv::Size(848, 480), CV_8UC3, (void *)color_frame.get_data(),
                    cv::Mat::AUTO_STEP);

      // Serialize the OpenCV image and publish it via ZeroMQ
      zmq::message_t zmq_msg(frame.total() * frame.elemSize());
      std::memcpy(zmq_msg.data(), frame.data, zmq_msg.size());
      publisher_->send(zmq_msg, zmq::send_flags::none);
    }
  }

private:
  rs2::pipeline pipeline_;
  std::shared_ptr<zmq::context_t> context_;
  std::shared_ptr<zmq::socket_t> publisher_;
};

#endif // __RGB_THREAD_H__