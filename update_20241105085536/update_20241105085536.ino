#include <micro_ros_arduino.h>
#include <mirs_msgs/srv/parameter_update.h>
#include <stdio.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int64.h>

rcl_node_t node;
rclc_support_t support;
rcl_allocator_t allocator;
rclc_executor_t executor;

rcl_service_t service;
rcl_wait_set_t wait_set;

mirs_msgs__srv__ParameterUpdate_Response res;
mirs_msgs__srv__ParameterUpdate_Request req;

#define PIN 32

void service_callback(const void * req, void * res){
  mirs_msgs__srv__ParameterUpdate_Request * req_in = (mirs_msgs__srv__ParameterUpdate_Request *) req;
  mirs_msgs__srv__ParameterUpdate_Response * res_in = (mirs_msgs__srv__ParameterUpdate_Response *) res;

  double a = req_in->wheel_radius + req_in->wheel_base + req_in->rkp + req_in->rki + req_in->rkd + req_in->lkp + req_in->lki + req_in->lkd;
  for(int i = 0; i < (int)a;i++){
    digitalWrite(PIN,HIGH);
    delay(500);
    digitalWrite(PIN,LOW);
    delay(500);
  }
  
  res_in->success = true;
}

void setup() {
  set_microros_transports();
  delay(1000); 

  allocator = rcl_get_default_allocator();

  // create init_options
  rclc_support_init(&support, 0, NULL, &allocator);

  // create node
  rclc_node_init_default(&node, "esp_test", "", &support);

  // create service
  rclc_service_init_default(&service, &node, ROSIDL_GET_SRV_TYPE_SUPPORT(mirs_msgs, srv, ParameterUpdate), "/update_parameters");

  // create executor
  rclc_executor_init(&executor, &support.context, 1, &allocator);
  rclc_executor_add_service(&executor, &service, &req, &res, service_callback);

  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
}


void loop() {
  delay(100);
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
}
