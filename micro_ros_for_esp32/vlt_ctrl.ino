void vlt_watch(){
  vlt_1 = analogRead(PIN_BATT);
}

void vlt_setup(){
  pinMode(PIN_BATT, ANALOG);

  enc_msg.data.size = 1; // メッセージ配列のサイズを3に設定
  enc_msg.data.data = (int32_t *)malloc(enc_msg.data.size * sizeof(int32_t)); // 配列のメモリを確保
  enc_msg.data.data[0] = 0;
}