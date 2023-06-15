#include <librealsense2/rs.hpp>
#include <opencv2/opencv.hpp>
#include <thread>
int main() {
  // Create a RealSense pipeline
  rs2::pipeline pipe;

  // Create a configuration for the pipeline
  rs2::config cfg;

  // Enable depth and color streams
  cfg.enable_stream(RS2_STREAM_DEPTH);
  cfg.enable_stream(RS2_STREAM_COLOR);

  // Start the pipeline
  pipe.start(cfg);

  // Wait for the camera to stabilize
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // Declare variables for calibration
  const int boardWidth = 9;  // Number of inner corners per row
  const int boardHeight = 8; // Number of inner corners per column
  const int numImages = 15;  // Number of calibration images to capture
  const float squareSize =
      0.0245f; // Size of each square on the calibration board in meters

  std::vector<std::vector<cv::Point2f>> imagePoints;
  std::vector<std::vector<cv::Point3f>> objectPoints;

  cv::Size boardSize(boardWidth, boardHeight);
  std::vector<cv::Point2f> corners;
  std::vector<cv::Point3f> obj;

  for (int i = 0; i < boardHeight; ++i) {
    for (int j = 0; j < boardWidth; ++j) {
      obj.emplace_back(j * squareSize, i * squareSize, 0);
    }
  }

  cv::Mat color, gray;

  int imageCount = 0;

  while (imageCount < numImages) {
    // Wait for a new frame from the camera
    rs2::frameset frames = pipe.wait_for_frames();

    // Get the color frame from the frameset
    rs2::video_frame colorFrame = frames.get_color_frame();

    // Convert the color frame to OpenCV format
    color = cv::Mat(cv::Size(colorFrame.get_width(), colorFrame.get_height()),
                    CV_8UC3, (void *)colorFrame.get_data(), cv::Mat::AUTO_STEP);

    // Convert the color image to grayscale
    cv::cvtColor(color, gray, cv::COLOR_BGR2GRAY);

    // Find chessboard corners in the grayscale image
    bool patternFound = cv::findChessboardCorners(gray, boardSize, corners,
                                                  cv::CALIB_CB_FAST_CHECK);

    if (patternFound) {
      // Refine the corner locations
      cv::cornerSubPix(
          gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
          cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30,
                           0.1));

      // Draw the corners on the color image
      cv::drawChessboardCorners(color, boardSize, corners, patternFound);

      // Display the color image with corners
      cv::imshow("Calibration", color);
      cv::waitKey(100);

      // Add the detected corners and corresponding object points
      imagePoints.push_back(corners);
      objectPoints.push_back(obj);

      imageCount++;

      std::cout << "Image " << imageCount << " captured for calibration."
                << std::endl;

      if (imageCount >= numImages) {
        break;
      }
    }
  }

  // Calibrate the camera
  cv::Mat cameraMatrix, distCoeffs;
  std::vector<cv::Mat> rvecs, tvecs;
  cv::calibrateCamera(objectPoints, imagePoints, color.size(), cameraMatrix,
                      distCoeffs, rvecs, tvecs);
  // Print the calibration results
  std::cout << "Camera matrix:" << std::endl << cameraMatrix << std::endl;
  std::cout << "Distortion coefficients:" << std::endl
            << distCoeffs << std::endl;

  // Save the calibration results to a file
  cv::FileStorage fs("calibration_results.xml", cv::FileStorage::WRITE);
  fs << "CameraMatrix" << cameraMatrix;
  fs << "DistCoeffs" << distCoeffs;
  fs.release();

  // Stop the pipeline
  pipe.stop();

  return 0;
}
