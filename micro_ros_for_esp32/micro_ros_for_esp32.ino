#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
//#include <TimeLib.h>
#include <PID_v1_bc.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <geometry_msgs/msg/transform_stamped.h>
#include <rosidl_runtime_c/string_functions.h>
#include <geometry_msgs/msg/twist.h>
#include <mirs_msgs/srv/parameter_update.h>
#include <mirs_msgs/srv/simple_command.h>
#include <mirs_msgs/msg/basic_param.h>
#include "define.h"
#include <builtin_interfaces/msg/time.h>
#include <tf2_msgs/msg/tf_message.h>
#include <micro_ros_utilities/type_utilities.h>
#include <micro_ros_utilities/string_utilities.h>
#include <std_msgs/msg/float64_multi_array.h>

std_msgs__msg__Int32MultiArray enc_msg;       //エンコーダー情報
std_msgs__msg__Float64MultiArray vlt_msg;     //電圧情報
std_msgs__msg__Float64MultiArray cuvel_msg;     //速度情報
geometry_msgs__msg__Twist vel_msg;            //速度指令値
mirs_msgs__msg__BasicParam param_msg;         //パラメーターメッセージ
mirs_msgs__srv__ParameterUpdate_Response update_res;
mirs_msgs__srv__ParameterUpdate_Request update_req;

rcl_publisher_t enc_pub;
rcl_publisher_t vlt_pub;
rcl_publisher_t cuvel_pub;
rcl_subscription_t cmd_vel_sub;
rcl_subscription_t param_sub;
rcl_service_t update_srv;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

int32_t count_l = 0;
int32_t count_r = 0;
int32_t last_count_l = 0;
int32_t last_count_r = 0;
// 速度計算用
double left_distance = 0;
double right_distance = 0;


// PID制御用の変数
double r_vel_cmd;
double l_vel_cmd;
double r_vel;
double l_vel;
double r_pwm;
double l_pwm;

double vlt_1 = 0;
double vlt_2 = 0;

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    //速度計算
    calculate_cuvel();

    //PID計算
    PID_control();
    //エンコーダーデータを格納
    enc_msg.data.data[0] = count_l;
    enc_msg.data.data[1] = count_r;
    //電圧観測
    //vlt_watch();
    vlt_msg.data.data[0] = l_vel;
    vlt_msg.data.data[1] = r_vel;
    rcl_publish(&enc_pub, &enc_msg, NULL);
    rcl_publish(&vlt_pub, &vlt_msg, NULL);
    rcl_publish(&cuvel_pub, &cuvel_msg, NULL);
  }
}

void setup() {
  set_microros_transports();

  //Serial.begin(115200); // シリアル通信を初期化

  encoder_open();

  delay(2000);

  //micro-ROSのセットアップ
  allocator = rcl_get_default_allocator();

  //rclc_support_init(&support, 0, NULL, &allocator);
  //rclc_node_init_default(&node, "ESP32_node", "", &support);

  //  nodeの作成とros_domein_idの作成
  rosid_setup_humble();
  //rosid_setup_foxy();

  //サブスクライバ、パブリッシャー、サービスの宣言
  rclc_publisher_init_default(
    &enc_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "/encoder"
  );

  rclc_publisher_init_default(
    &vlt_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float64MultiArray),
    "/vlt"
  );

  rclc_publisher_init_default(
    &cuvel_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float64MultiArray),
    "/vel"
  );

  rclc_subscription_init_default(
    &cmd_vel_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "/cmd_vel"
  );
  
  rclc_subscription_init_default(
    &param_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(mirs_msgs, msg, BasicParam),
    "/params"
  );
  
  rclc_service_init_default(
    &update_srv,
    &node,
    ROSIDL_GET_SRV_TYPE_SUPPORT(mirs_msgs, srv, ParameterUpdate),
    "/esp_update"
  );

  const uint32_t timer_timeout = 50;

  rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback
  );

  rclc_executor_init(&executor, &support.context, 4, &allocator);
  rclc_executor_add_subscription(&executor, &cmd_vel_sub, &vel_msg, &cmd_vel_Callback, ON_NEW_DATA);
  rclc_executor_add_subscription(&executor, &param_sub, &param_msg, &param_Callback, ON_NEW_DATA);
  rclc_executor_add_service(&executor, &update_srv, &update_req, &update_res, update_service_callback);
  rclc_executor_add_timer(&executor, &timer);

  cmd_vel_set();
  vlt_setup();
  cuvel_setup();

  delay(2000);

}

void loop() {
  delay(10);
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
}
