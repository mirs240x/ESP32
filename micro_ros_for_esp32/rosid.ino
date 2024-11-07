/*    ROS_DOMAIN_ID 設定用                  */
/*    foxyの場合はrosid_setup_foxy()        */
/*    humbleの場合はrosid_setup_humble()    */

rcl_node_options_t node_ops; // Foxy
rcl_init_options_t init_options; // Humble
#define ROS_DOMAIN_ID 90

void rosid_setup_foxy(){
  node_ops = rcl_node_get_default_options();
  //node_ops.domain_id = ROS_DOMAIN_ID;
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_with_options(&node, "ESP32_node", "", &support, &node_ops);
}

void rosid_setup_humble(){
  init_options = rcl_get_zero_initialized_init_options();
  size_t domain_id = ROS_DOMAIN_ID;
  rcl_init_options_init(&init_options, allocator); // <--- This was missing on ur side
  // Set ROS domain id
  rcl_init_options_set_domain_id(&init_options, domain_id);
  // Setup support structure.
  rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
  // create node
  rclc_node_init_default(&node, "ESP32_node", "", &support);
}
