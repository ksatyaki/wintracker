#include <wintracker/Synchronize.h>
#include <tf/transform_listener.h>
#include <tf_conversions/tf_eigen.h>
#include <eigen_conversions/eigen_msg.h>

class WinRebaseNode {

private:
  // Our NodeHandle, points to home
  ros::NodeHandle nh_;
  ros::NodeHandle n_;
  Eigen::Affine3d wrist2body, win2base, reference;
	
  //ros::Publisher gripper_map_publisher_;
  ros::Publisher pub_pose;
  ros::Subscriber sub_pose;
  ros::ServiceServer srv_synchronize_;
  tf::TransformListener tl;

  std::string wrist_frame, base_frame;
  std::string wintracker_topic, rebased_topic;

  bool is_set, is_sim_time;

  //DEBUG
  //ros::Publisher pub_old_pose;
  //ros::Publisher pub_emitter_pose;
  //END DEBUG

public:
  WinRebaseNode() {

    is_set = false;
    nh_ = ros::NodeHandle("~");
    n_ = ros::NodeHandle();

    nh_.param<std::string>("wrist_frame",wrist_frame,"gripper_r_base");
    nh_.param<std::string>("base_frame",base_frame,"yumi_body");
    nh_.param<std::string>("wintracker_topic",wintracker_topic,"/wintracker/pose");
    nh_.param<std::string>("rebased_topic",rebased_topic,"pose");
    //n_.param<bool>("use_sim_time",is_sim_time,false);
    is_sim_time = true;

    //	    
    srv_synchronize_=nh_.advertiseService("synchronize",&WinRebaseNode::synchronize,this);
    //
  }
        
  bool synchronize(wintracker::Synchronize::Request  &req, wintracker::Synchronize::Response &res)
  {
    is_set=false;
    sub_pose.shutdown();
    pub_pose.shutdown();
    sub_pose = n_.subscribe(wintracker_topic, 5, &WinRebaseNode::poseCallback, this);
    pub_pose = nh_.advertise<geometry_msgs::PoseStamped> (rebased_topic,10);
    //DEBUG
    //pub_old_pose = nh_.advertise<geometry_msgs::PoseStamped> ("old_pose",10);
    //pub_emitter_pose = nh_.advertise<geometry_msgs::PoseStamped> ("emitter_pose",10);
    //END DEBUG

    res.success=true;
    return res.success;
  }	

  void poseCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
  {
    if(!is_set) {
      ROS_INFO("Got first message, computing transform");
      //set the reference transform
      tf::StampedTransform wrist_to_body;
      try {
	ros::Time t_now;
	if(is_sim_time) {
	  t_now = ros::Time::now();
	} else {
	  t_now = msg->header.stamp;
	}
	tl.waitForTransform(base_frame, wrist_frame, t_now, ros::Duration(1.) );
	tl.lookupTransform(base_frame, wrist_frame, t_now, wrist_to_body);
      } catch (tf::TransformException ex) {
	ROS_ERROR("%s",ex.what());
	return;
      }
      tf::transformTFToEigen(wrist_to_body,wrist2body);
      tf::poseMsgToEigen(msg->pose, win2base);

      reference = wrist2body*win2base.inverse();
      std::cout<<reference.matrix()<<std::endl;
      is_set = true;
    }
    else {
      //publish the rebased pose
      Eigen::Affine3d current;
      tf::poseMsgToEigen(msg->pose, current);
      current = reference*current;

      geometry_msgs::PoseStamped newmsg;
      newmsg.header.stamp = msg->header.stamp;
      newmsg.header.frame_id = base_frame;
      newmsg.header.seq = msg->header.seq;

      tf::poseEigenToMsg(current, newmsg.pose);
      pub_pose.publish(newmsg);
  

      //DEBUG
      //publish the old pose expressed in the base frame
      //newmsg.pose=msg->pose;
      //pub_old_pose.publish(newmsg);
      //publish the emitter expressed in the base frame
      //tf::poseEigenToMsg(reference, newmsg.pose);
      //pub_emitter_pose.publish(newmsg);
      //END DEBUG

    }
  }

};

int main(int argc, char **argv) {

  ros::init(argc,argv,"wintracker_rebase");
  std::cout<<"Starting up...\n"; 
  ROS_INFO("Starting Wintracker rebase node.");
  WinRebaseNode nd;
  // ros::AsyncSpinner spinner(4); // Use 4 threads
  // spinner.start();
  ros::spin();
  ros::waitForShutdown();
  ROS_INFO("Wintracker rebase node stopped.");
  return 0;
}
