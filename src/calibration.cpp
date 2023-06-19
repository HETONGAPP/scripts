#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>

int main() {
  // Declare RealSense pipeline and configuration
  rs2::pipeline pipe;
  rs2::config cfg;

  // Enable depth and infrared streams
  cfg.enable_stream(RS2_STREAM_DEPTH);
  cfg.enable_stream(RS2_STREAM_INFRARED);

  // Start the pipeline
  pipe.start(cfg);

  // Wait for the auto-calibration to complete
  rs2::frameset frames;
  while (true) {
    frames = pipe.wait_for_frames();
    if (frames.get_depth_frame().get_frame_metadata(
            RS2_FRAME_METADATA_FRAME_TIMESTAMP) != 0) {
      // Calibration is complete
      break;
    }
  }

  // Retrieve the auto-calibrated camera parameters
  rs2_intrinsics intrinsics = frames.get_depth_frame()
                                  .get_profile()
                                  .as<rs2::video_stream_profile>()
                                  .get_intrinsics();
  rs2::stream_profile infrared_stream_profile =
      frames.get_infrared_frame().get_profile();
  rs2_extrinsics extrinsics = frames.get_depth_frame()
                                  .get_profile()
                                  .as<rs2::video_stream_profile>()
                                  .get_extrinsics_to(infrared_stream_profile);

  // Save the calibration parameters to an XML file
  cv::FileStorage fs("calibration_parameters.xml", cv::FileStorage::WRITE);
  fs << "CalibrationParameters"
     << "{";
  fs << "Width" << intrinsics.width;
  fs << "Height" << intrinsics.height;
  fs << "FocalLengthX" << intrinsics.fx;
  fs << "FocalLengthY" << intrinsics.fy;
  fs << "PrincipalPointX" << intrinsics.ppx;
  fs << "PrincipalPointY" << intrinsics.ppy;
  fs << "RotationMatrix" << cv::Mat(3, 3, CV_64F, extrinsics.rotation);
  fs << "TranslationVector" << cv::Mat(3, 1, CV_64F, extrinsics.translation);
  fs << "}";
  fs.release();

  // Print a success message
  std::cout << "Calibration parameters saved to 'calibration_parameters.xml'."
            << std::endl;

  // Stop the pipeline
  pipe.stop();

  return 0;
}
