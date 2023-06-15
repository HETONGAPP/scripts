#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <zmq.hpp>

int main() {
  zmq::context_t context(1);
  zmq::socket_t publisher(context, ZMQ_PUB);
  publisher.bind("tcp://*:5555"); // Change the address as needed

  rs2::pipeline pipe;
  rs2::config cfg;
  cfg.enable_stream(RS2_STREAM_COLOR, 848, 480, RS2_FORMAT_BGR8, 30);
  pipe.start(cfg);

  while (true) {
    rs2::frameset frames = pipe.wait_for_frames();
    rs2::frame color_frame = frames.get_color_frame();
    cv::Mat frame(cv::Size(848, 480), CV_8UC3, (void *)color_frame.get_data(),
                  cv::Mat::AUTO_STEP);

    // Serialize the OpenCV image and publish it via ZeroMQ
    zmq::message_t zmq_msg(frame.total() * frame.elemSize());
    std::memcpy(zmq_msg.data(), frame.data, zmq_msg.size());
    publisher.send(zmq_msg, zmq::send_flags::none);
    // Display the frame (optional)
    cv::imshow("RealSense Feed", frame);
    if (cv::waitKey(1) == 27) // Press ESC to exit
      break;
  }

  pipe.stop();
  return 0;
}
