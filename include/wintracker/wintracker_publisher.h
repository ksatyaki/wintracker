/** \file wintracker_node.h
 * \author Krzysztof Charusta 
 *
 * 
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2008, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *Todo: implement the hardware filtering routines
 *
 */

#ifndef WINTRACKER_PUBLISHER_H
#define WINTRACKER_PUBLISHER_H

#include <ros/ros.h>
#include <string.h>
#include <wintracker/GetPose.h>
#include <boost/thread/mutex.hpp>
#include <Eigen/Core>
#include <geometry_msgs/PoseStamped.h>
#include <deque>

#define JITTER_WINDOW 5

/** \brief interface to wintracker 6d pose tracker
 *
 * Class is an interface to wintracker 6d pose. Publishes two topics:
 * Pose (pub_), and PoseStamped (pubTest_). The header_frame_id_ can be
 * specified via a launch file
 *
 * Note: To run this publisher without sudo privilates you need to add
 * a wintracker rule to udev. Details how to do that are given in the README.
 * 
 */
class WintrackerPublisher {
  
 public:
  WintrackerPublisher();
  virtual ~WintrackerPublisher();

  void startWTracker();
  void startStreaming();
  bool spin();
  
 private:

  ros::NodeHandle nh_;
  ros::Publisher pub_;
  ros::ServiceServer pose_srv_;
  std::string frame_id_;
  boost::mutex data_mutex_;
  std::string hemisphere_;
  Eigen::Matrix<int,7,Eigen::Dynamic> sign_buffer_;
  Eigen::Matrix<float,7,1> posture_ref_;

   /* Generates a new pose performing the sign-filtering  */
  geometry_msgs::PoseStamped getFilteredTick(); 

  /* The WinTracker often incorrectly flips signs of the sensor readings in case of
   *  disturbances. The posture is frozen until all signs in the sign_buffer_ are the same
   */
  void filterSignJitter(Eigen::Vector3f& pos, Eigen::Vector4f& ori);


  //Helper functions
  Eigen::Vector3f getCurrPos();
  Eigen::Vector4f getCurrOri();
  bool signsEqual(Eigen::VectorXi const& vec);
  int signof(float val);
  Eigen::VectorXi signof(Eigen::VectorXf vec);


  /////////////////
  //  CALLBACKS  //
  /////////////////
 
  bool getPose(wintracker::GetPose::Request  &req, wintracker::GetPose::Response &res);

};

#endif
