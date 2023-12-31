#ifndef __PCL_THREAD_H__
#define __PCL_THREAD_H__

#include <Eigen/Dense>
#include <chrono>
#include <iostream>
#include <librealsense2/rs.hpp>
#include <nlohmann/json.hpp>
#include <opencv2/highgui.hpp>
#include <pcl/filters/filter.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/radius_outlier_removal.h>
#include <pcl/filters/random_sample.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/ransac.h>
#include <pcl/visualization/cloud_viewer.h>
#include <vector>
#include <zmq.hpp>

using PointT = pcl::PointXYZRGB;
using PointCloudT = pcl::PointCloud<PointT>;
using cloud_pointer = PointCloudT::Ptr;
using prevCloud = PointCloudT::Ptr;

class PCLStream {

public:
  PCLStream(rs2::pipeline &pipe) : pipeline_(pipe) {
    context_ = std::make_shared<zmq::context_t>(1);
    publisher_ = std::make_shared<zmq::socket_t>(*context_, ZMQ_PUB);
    publisher_->bind("tcp://*:5558");
  }
  ~PCLStream() {
    pipeline_.stop();
    context_->close();
    publisher_->close();
  }
  std::tuple<int, int, int> RGB_Texture(rs2::video_frame texture,
                                        rs2::texture_coordinate Texture_XY) {
    // Get Width and Height coordinates of texture
    int width = texture.get_width();   // Frame width in pixels
    int height = texture.get_height(); // Frame height in pixels

    // Normals to Texture Coordinates conversion
    int x_value =
        std::min(std::max(int(Texture_XY.u * width + .5f), 0), width - 1);
    int y_value =
        std::min(std::max(int(Texture_XY.v * height + .5f), 0), height - 1);

    int bytes =
        x_value * texture.get_bytes_per_pixel(); // Get # of bytes per pixel
    int strides =
        y_value * texture.get_stride_in_bytes(); // Get line width in bytes
    int Text_Index = (bytes + strides);

    const auto New_Texture =
        reinterpret_cast<const uint8_t *>(texture.get_data());

    // RGB components to save in tuple
    int NT1 = New_Texture[Text_Index];
    int NT2 = New_Texture[Text_Index + 1];
    int NT3 = New_Texture[Text_Index + 2];

    return std::tuple<int, int, int>(NT1, NT2, NT3);
  }

  //===================================================
  //  PCL_Conversion
  // - Function is utilized to fill a point cloud object with depth and RGB data
  // from a single frame captured using the Realsense.
  //===================================================
  cloud_pointer PCL_Conversion(const rs2::points &points,
                               const rs2::video_frame &color) {

    // Object Declaration (Point Cloud)
    cloud_pointer cloud(new PointCloudT);

    // Declare Tuple for RGB value Storage (<t0>, <t1>, <t2>)
    std::tuple<uint8_t, uint8_t, uint8_t> RGB_Color;

    //================================
    // PCL Cloud Object Configuration
    //================================
    // Convert data captured from Realsense camera to Point Cloud
    auto sp = points.get_profile().as<rs2::video_stream_profile>();

    cloud->width = static_cast<uint32_t>(sp.width());
    cloud->height = static_cast<uint32_t>(sp.height());
    cloud->is_dense = false;
    cloud->points.resize(points.size());

    auto Texture_Coord = points.get_texture_coordinates();
    auto Vertex = points.get_vertices();

    // Iterating through all points and setting XYZ coordinates
    // and RGB values
    for (int i = 0; i < points.size(); i++) {
      //===================================
      // Mapping Depth Coordinates
      // - Depth data stored as XYZ values
      //===================================
      cloud->points[i].x = Vertex[i].x;
      cloud->points[i].y = Vertex[i].y;
      cloud->points[i].z = Vertex[i].z;

      // Obtain color texture for specific point
      RGB_Color = RGB_Texture(color, Texture_Coord[i]);

      // Mapping Color (BGR due to Camera Model)
      cloud->points[i].r = std::get<2>(RGB_Color); // Reference tuple<2>
      cloud->points[i].g = std::get<1>(RGB_Color); // Reference tuple<1>
      cloud->points[i].b = std::get<0>(RGB_Color); // Reference tuple<0>
    }

    return cloud; // PCL RGB Point Cloud generated
  }
  void Start() {
    while (true) {
      // Wait for the next set of frames
      rs2::frameset frames = pipeline_.wait_for_frames();

      // Get the depth frame
      rs2::depth_frame depth_frame = frames.get_depth_frame();

      rs2::video_frame color_frame = frames.get_color_frame();

      if (!depth_frame || !color_frame) {
        // Handle the case where one or both frames are invalid
        std::cerr << "Invalid frames received" << std::endl;
        continue;
      }
      rs2::pointcloud pc;
      pc.map_to(color_frame);
      rs2::points points = pc.calculate(depth_frame);

      PointCloudT::Ptr cloud(new PointCloudT);

      cloud = PCL_Conversion(points, color_frame);

      pcl::PassThrough<PointT> pass;
      pass.setInputCloud(cloud);
      pass.setFilterFieldName("z");
      pass.setFilterLimits(0.0, 1.0);

      PointCloudT::Ptr cloud_filtered(new PointCloudT);
      pass.filter(*cloud_filtered);

      PointCloudT::Ptr cloud_downsampled(new PointCloudT);
      pcl::RandomSample<PointT> rs;

      if (cloud_filtered->size() >= 300) {
        rs.setInputCloud(cloud_filtered);
        rs.setSample(300);
        rs.filter(*cloud_downsampled);
      }
      nlohmann::json json_obj;
      for (auto point : cloud_downsampled->points) {
        nlohmann::json point_obj;
        point_obj["x"] = point.x;
        point_obj["y"] = point.y;
        point_obj["z"] = point.z;
        json_obj.push_back(point_obj);
      }
      std::string json_str = json_obj.dump();
      // std::cout << json_str << std::endl;
      zmq::message_t message(json_str.size());
      memcpy(message.data(), json_str.data(), json_str.size());
      publisher_->send(message);
    }
  }

private:
  rs2::pipeline pipeline_;
  std::shared_ptr<zmq::context_t> context_;
  std::shared_ptr<zmq::socket_t> publisher_;
};
#endif // __PCL_THREAD_H__