/* ROS_DOMAIN_ID 設定用 */

rcl_node_options_t node_ops; // Foxy
rcl_init_options_t init_options; // Humble
size_t domain_id = 89;

void rosid_setup_foxy(){
  rclc_support_init(&support, 0, NULL, &allocator);
  rclc_node_init_with_options(&node, "ESP32_node", "", &support, &node_ops);
}

void rosid_setup_humble(){
  init_options = rcl_get_zero_initialized_init_options();
  rcl_init_options_init(&init_options, allocator); // <--- This was missing on ur side
  // Set ROS domain id
  rcl_init_options_set_domain_id(&init_options, domain_id);
  // Setup support structure.
  rclc_support_init_with_options(&support, 0, NULL, &init_options, &allocator);
  // create node
  rclc_node_init_default(&node, "ESP32_node", "", &support);
}

  