/*
 * FiltersDemo.cpp
 *
 *  Created on: Aug 16, 2017
 *      Author: Peter Fankhauser
 *   Institute: ETH Zurich, ANYbotics
 *
 */

#include "grid_map_demos/FiltersDemo.hpp"

using namespace grid_map;

namespace grid_map_demos {

FiltersDemo::FiltersDemo(ros::NodeHandle& nodeHandle, bool& success)
    : nodeHandle_(nodeHandle),
      filterChain_("grid_map::GridMap")
{
  if (!readParameters()) {
    success = false;
    return;
  }

  subscriber_ = nodeHandle_.subscribe(inputTopic_, 1, &FiltersDemo::callback, this);
  publisher_ = nodeHandle_.advertise<grid_map_msgs::GridMap>(outputTopic_, 1, true);
  server_ = nodeHandle_.advertiseService("reload_filters", &FiltersDemo::reloadServiceCallback, this);

  // Setup filter chain.
  if (!filterChain_.configure(filterChainParametersName_, nodeHandle)) {
    ROS_ERROR("Could not configure the filter chain!");
    success = false;
    return;
  }

  success = true;
}

FiltersDemo::~FiltersDemo()
{
}

bool FiltersDemo::readParameters()
{
  if (!nodeHandle_.getParam("input_topic", inputTopic_)) {
    ROS_ERROR("Could not read parameter `input_topic`.");
    return false;
  }
  nodeHandle_.param("output_topic", outputTopic_, std::string("output"));
  nodeHandle_.param("filter_chain_parameter_name", filterChainParametersName_, std::string("grid_map_filters"));
  return true;
}

void FiltersDemo::callback(const grid_map_msgs::GridMap& message)
{
  // Convert message to map.
  GridMap inputMap;
  GridMapRosConverter::fromMessage(message, inputMap);

  // Apply filter chain.
  grid_map::GridMap outputMap;
  if (!filterChain_.update(inputMap, outputMap)) {
    ROS_ERROR("Could not update the grid map filter chain!");
    return;
  }

  ROS_INFO("PUBLISH");
  // Publish filtered output grid map.
  grid_map_msgs::GridMap outputMessage;
  GridMapRosConverter::toMessage(outputMap, outputMessage);
  publisher_.publish(outputMessage);
}

bool FiltersDemo::reloadServiceCallback(std_srvs::Empty::Request& req, std_srvs::Empty::Response& resp)
{
  if (!readParameters()) {
    return false;
  }

  // nodeHandle_.param("filter_chain_parameter_name", filterChainParametersName_, std::string("grid_map_filters"));
  if (!filterChain_.configure(filterChainParametersName_, nodeHandle_))
  {
    ROS_ERROR("Could not configure the filter chain!");
    return false;
  }

  return true;
}

} /* namespace */
