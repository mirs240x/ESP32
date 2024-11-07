void vlt_watch(){
  vlt_1 = analogRead(PIN_BATT);
}

void vlt_setup(){
  pinMode(PIN_BATT, ANALOG);

  vlt_msg.data.size = 1; // メッセージ配列のサイズを3に設定
  vlt_msg.data.data = (double *)malloc(enc_msg.data.size * sizeof(double)); // 配列のメモリを確保
  vlt_msg.data.data[0] = 0;
}