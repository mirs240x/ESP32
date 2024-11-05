#include <micro_ros_arduino.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <PID_v1_bc.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/int32_multi_array.h>
#include <nav_msgs/msg/odometry.h>
#include <geometry_msgs/msg/transform_stamped.h>
#include <rosidl_runtime_c/string_functions.h>
#include <tf2_msgs/msg/tf_message.h>
#include <geometry_msgs/msg/twist.h>
#include <mirs_msgs/srv/parameter_update.h>
#include <mirs_msgs/srv/simple_command.h>
#include "quaternion.h"
#include "define.h"
#include <builtin_interfaces/msg/time.h>

nav_msgs__msg__Odometry odom_msg;             //オドメトリ
geometry_msgs__msg__TransformStamped odom_tf; //tf変換
std_msgs__msg__Int32MultiArray enc_msg;       //エンコーダー情報
geometry_msgs__msg__Twist vel_msg;            //速度指令値
mirs_msgs__srv__ParameterUpdate_Response update_res;
mirs_msgs__srv__ParameterUpdate_Request update_req;
//mirs_msgs__srv__SimpleCommand_Response reboot_res;
//mirs_msgs__srv__SimpleCommand_Request reboot_req;
//mirs_msgs__srv__SimpleCommand_Response reset_res;
//mirs_msgs__srv__SimpleCommand_Request reset_req;

rcl_publisher_t odom_pub;
rcl_publisher_t tf_broadcaster;
rcl_publisher_t enc_pub;
rcl_subscription_t cmd_vel_sub;
rcl_service_t update_srv;
rcl_service_t reboot_srv;
rcl_service_t reset_srv;
rcl_wait_set_t wait_set;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

int32_t count_l,count_r;
int32_t last_count_l,last_count_r;

double left_distance;
double right_distance;

// PID制御用の変数
double r_vel_cmd;
double l_vel_cmd;
double r_vel;
double l_vel;
double r_pwm;
double l_pwm;

int32_t abc,def;

const int timeout_ms = 1000;
static int64_t time_ms;
static time_t time_seconds;


void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    //オドメトリ計算
    calculate_odometry();
    //PID計算
    PID_control();
    //エンコーダーデータを格納
    enc_msg.data.data[0] = abc;
    enc_msg.data.data[1] = def;
    rcl_publish(&enc_pub, &enc_msg, NULL);
    rcl_publish(&odom_pub, &odom_msg, NULL);
    rcl_publish(&tf_broadcaster, &odom_tf, NULL);
  }
}

void setup() {
  Serial.begin(115200); // シリアル通信を初期化
  encoder_open();
  set_microros_transports();
  delay(2000);

  //micro-ROSのセットアップ
  allocator = rcl_get_default_allocator();

  rclc_support_init(&support, 0, NULL, &allocator);

  rclc_node_init_default(&node, "ESP32_node", "", &support);

  //サブスクライバとパブリッシャーの宣言
  rclc_publisher_init_default(
    &odom_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(nav_msgs, msg, Odometry),
    "/odom"
  );

  rclc_publisher_init_default(
    &tf_broadcaster,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, TransformStamped),
    "/tf"
  );

  rclc_publisher_init_default(
    &enc_pub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32MultiArray),
    "/encoder"
  );

  rclc_subscription_init_default(
    &cmd_vel_sub,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(geometry_msgs, msg, Twist),
    "/cmd_vel"
  );

  // サービスの宣言
  rclc_service_init_default(
    &update_srv,
    &node,
    ROSIDL_GET_SRV_TYPE_SUPPORT(mirs_msgs, srv, ParameterUpdate),
    "/esp_update"
  );
  /*
  rclc_service_init_default(
    &reboot_srv,
    &node,
    ROSIDL_GET_SRV_TYPE_SUPPORT(mirs_msgs, srv, SimpleCommand),
    "/esp_reboot"
  );

  rclc_service_init_default(
    &reset_srv,
    &node,
    ROSIDL_GET_SRV_TYPE_SUPPORT(mirs_msgs, srv, SimpleCommand),
    "/esp_reset"
  );
  */
  const uint32_t timer_timeout = 100;

  rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback
  );

  rclc_executor_init(&executor, &support.context, 3, &allocator);
  rclc_executor_add_subscription(&executor, &cmd_vel_sub, &vel_msg, &cmd_vel_Callback, ON_NEW_DATA);
  rclc_executor_add_service(&executor, &update_srv, &update_req, &update_res, update_service_callback);
  //rclc_executor_add_service(&executor, &reboot_srv, &reboot_req, &reboot_res, reboot_service_callback);
  //rclc_executor_add_service(&executor, &reset_srv, &reset_req, &reset_res, reset_service_callback);
  rclc_executor_add_timer(&executor, &timer);

  odometry_set();
  cmd_vel_set();


  delay(2000);

}

void loop() {
  delay(10);
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100));
}
