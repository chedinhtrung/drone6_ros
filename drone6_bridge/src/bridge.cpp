#include <cmath>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"

#include "drone6_bridge/message_helpers.h"
#include "drone6_bridge/fc_uart.h"

using namespace std::chrono_literals;

class BridgeNode : public rclcpp::Node
{
public:
  BridgeNode() : Node("fake_odom_node")
  {
    msg_parser_ = RPiFrameParser();
    pub_ = create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    timer_ = create_wall_timer(10ms, [this]()
    {
      // read from the UART port
      uint8_t new_byte;
      bool got_frame = false;
      while (uart_.works && this->uart_.read_byte(new_byte))
      {
        if (this->msg_parser_.parse_byte(new_byte))
        {
          got_frame = true;
          break;
        };
      } 
      if (got_frame && msg_parser_.type == RPiMessageType::STATE){
        ESKFStatePayload pl;
        if(this->msg_parser_.unpack_payload(pl)){
          NominalState n = unpack(pl);
          publish(n);
        };
      }
      //TODO: other message types from the FC
    }
    );
  }

private:
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
  rclcpp::TimerBase::SharedPtr timer_;
  RPiFrameParser msg_parser_;
  FC_UART uart_{};

  void publish(const NominalState& n)
  {
    nav_msgs::msg::Odometry msg;
    msg.header.stamp = now();
    msg.header.frame_id = "odom";
    msg.child_frame_id = "base_link";
    msg.pose.pose.position.x = n.p.x;
    msg.pose.pose.position.y = n.p.y;
    msg.pose.pose.position.z = n.p.z;

    msg.twist.twist.linear.x = n.v.x;
    msg.twist.twist.linear.y = n.v.y;
    msg.twist.twist.linear.z = n.v.z;

    msg.pose.pose.orientation.w = n.q.w;
    msg.pose.pose.orientation.x = n.q.x;
    msg.pose.pose.orientation.y = n.q.y;
    msg.pose.pose.orientation.z = n.q.z;

    geometry_msgs::msg::TransformStamped tf_msg;

    tf_msg.header.stamp = msg.header.stamp;
    tf_msg.header.frame_id = "odom";
    tf_msg.child_frame_id = "base_link";

    tf_msg.transform.translation.x = msg.pose.pose.position.x;
    tf_msg.transform.translation.y = msg.pose.pose.position.y;
    tf_msg.transform.translation.z = msg.pose.pose.position.z;

    tf_msg.transform.rotation = msg.pose.pose.orientation;

    tf_broadcaster_->sendTransform(tf_msg);

    pub_->publish(msg);
  }
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BridgeNode>());
  rclcpp::shutdown();
  return 0;
}
