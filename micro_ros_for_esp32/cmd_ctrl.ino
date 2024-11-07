// PWM設定
const int32_t r_Channel = 0;        // PWMチャンネル
const int32_t l_Channel = 1;
const int32_t pwmFrequency = 5000; // PWM周波数 (5kHz)
const int32_t pwmResolution = 8;   // PWM分解能 (8ビット = 0-255)

float linear_x;   //  直進速度
float angular_z;  //  回転速度

float r_err_sum = 0;
float l_err_sum = 0;

float dist_r_err = 0;
float dist_l_err = 0;

// cmd_velメッセージのコールバック関数
void cmd_vel_Callback(const void * msgin) {
  const geometry_msgs__msg__Twist * vel_msg = (const geometry_msgs__msg__Twist *)msgin;

  // linear.x と angular.z のデータを取得
  linear_x = vel_msg->linear.x;
  angular_z = vel_msg->angular.z;

  //  目標速度計算
  r_vel_cmd = linear_x + WHEEL_BASE / 2 * angular_z;
  l_vel_cmd = linear_x - WHEEL_BASE / 2 * angular_z;
}

void PID_control(){

  float r_err = r_vel_cmd - r_vel;
  float l_err = l_vel_cmd - l_vel;

  r_err_sum += r_err;
  l_err_sum += l_err;

  // PID計算を実行
  r_pwm = RKP * r_err + RKI * r_err_sum + RKD *  (r_err - dist_r_err);
  l_pwm = LKP * l_err + LKI * l_err_sum + LKD *  (l_err - dist_l_err);

  dist_r_err = r_err;
  dist_l_err = l_err;

  if(r_pwm > 255){
    r_pwm = 255;
  }else if(r_pwm < -255){
    l_pwm = -255;
  }
  if(l_pwm > 255){
    l_pwm = 255;
  }else if(l_pwm < -255){
    l_pwm = -255;
  }

  //  出力値の前処理
  if(r_pwm >= 0){
    digitalWrite(PIN_DIR_R, LOW);
  }else{
    r_pwm *= -1;
    digitalWrite(PIN_DIR_R,HIGH);
  }
  if(l_pwm >= 0){
    digitalWrite(PIN_DIR_L, HIGH);
  }else{
    l_pwm *= -1;
    digitalWrite(PIN_DIR_L,LOW);
  }

  //  確実な停止
  if(r_vel_cmd == 0){
    r_pwm = 0;
  }
  if(l_vel_cmd == 0){
    l_pwm = 0; 
  }

  //  pwm出力
  ledcWrite(r_Channel, uint8_t(r_pwm));
  ledcWrite(l_Channel, uint8_t(l_pwm));
}

void cmd_vel_set() {
  // ledcのPWM設定
  pinMode(PIN_DIR_R, OUTPUT);
  pinMode(PIN_DIR_L, OUTPUT);
  ledcSetup(r_Channel, pwmFrequency, pwmResolution);
  ledcSetup(l_Channel, pwmFrequency, pwmResolution);
  ledcAttachPin(PIN_PWM_R, r_Channel);
  ledcAttachPin(PIN_PWM_L, l_Channel);
}
