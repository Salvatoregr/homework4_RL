#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <geometry_msgs/PoseStamped.h>
#include "boost/thread.hpp"
#include "Eigen/Dense"
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>

class TF_NAV {

    public:
        TF_NAV();
        void run();
        void tf_listener_fun();
        void position_pub();
        void goal_listener();
        void send_goal();
        void tf_aruco_fun();
        void publishArUcoTF(const tf::Vector3&, const tf::Quaternion&);
        void publishTF(const tf::Vector3&, const tf::Quaternion&);

    private:

        ros::NodeHandle _nh;

        ros::Publisher _position_pub;
        ros::Publisher _pos_pub;

        Eigen::Vector3d _home_pos;

        Eigen::Vector3d _cur_pos;
        Eigen::Vector4d _cur_or;

        Eigen::Vector3d _goal_pos;
        Eigen::Vector4d _goal_or;
        
        Eigen::Vector3d _aruco_pos;
        Eigen::Vector4d _aruco_or;
        typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;


};
