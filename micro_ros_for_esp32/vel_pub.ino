void calculate_cuvel(){
  //エンコーダーの変化量を計算
  int32_t delta_left = count_l - last_count_l;
  int32_t delta_right = count_r - last_count_r;

  last_count_l = count_l;
  last_count_r = count_r;

  // 車輪の回転角度（ラジアン）を計算
  double delta_left_rad = (delta_left / COUNTS_PER_REV) * 2.0 * PI;
  double delta_right_rad = (delta_right / COUNTS_PER_REV) * 2.0 * PI;

  // それぞれの車輪の移動距離を計算
  left_distance = delta_left_rad * WHEEL_RADIUS;
  right_distance = delta_right_rad * WHEEL_RADIUS;

  // 現在の速度を計算
  l_vel = left_distance / 0.05;
  r_vel = right_distance / 0.05;

  cuvel_msg.data.data[0] = l_vel;
  cuvel_msg.data.data[1] = r_vel;
}

void cuvel_setup(){
  cuvel_msg.data.size = 2; // メッセージ配列のサイズを3に設定
  cuvel_msg.data.data = (double *)malloc(enc_msg.data.size * sizeof(double)); // 配列のメモリを確保
  cuvel_msg.data.data[0] = 0;
  cuvel_msg.data.data[1] = 0;
}
