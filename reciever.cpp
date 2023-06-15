#include <opencv2/opencv.hpp>
#include <zmq.hpp>

int main()
{
    zmq::context_t context(1);
    zmq::socket_t subscriber(context, ZMQ_SUB);
    subscriber.connect("tcp://localhost:5555");  // Change the address as needed
    subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while (true)
    {
        zmq::message_t zmq_msg;
        subscriber.recv(&zmq_msg);

        // Deserialize the received message into an OpenCV image
        cv::Mat image(480, 640, CV_8UC3, zmq_msg.data());

        // Show the received image
        cv::imshow("Received Image", image);
        cv::waitKey(1);
    }

    return 0;
}


