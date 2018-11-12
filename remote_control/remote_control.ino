
unsigned char arr[5] = {};

int LX = 0;
int LY = 0;
int LK = 0;
int RX = 0;
int RY = 0;
int RK = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  //  设置模式开启
  //  pinMode(20, OUTPUT);
  //  digitalWrite(20, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:

  //左遥杆信息
  LX = analogRead(A0);
  LY = analogRead(A1);
  LK = digitalRead(A2);
  //右遥杆信息
  RX = analogRead(A13);
  RY = analogRead(A14);
  RK = digitalRead(A15);

  Serial.print("   LX:");
  Serial.print(LX);
  Serial.print("   LY:");
  Serial.print(LY);
  Serial.print("   LK:");
  Serial.print(LK);
  Serial.print("-- RX:");
  Serial.print(RX);
  Serial.print("   RY:");
  Serial.print(RY);
  Serial.print("   RK:");
  Serial.println(RK);

  arr[0] = format(LX);
  arr[1] = format(LY);
  arr[2] = format(RX);
  arr[3] = format(RY);
  arr[4] = btnPress(LK, RK);


  //发送数据
  //    Serial1.print("hello#");
  Serial1.write(arr, 5);

  //设置模式
  //   Serial1.print("AT+C200\r\n");
  //   Serial1.print("AT+RX\r\n");
  //   String str = Serial1.readStringUntil('3');
  //   Serial.println(str);
  delay(80);
}

int format(int num) {
  if (num > 500 && num < 530)
  {
    return 127;
  }
  else
  {
    return map(num, 10, 1020, 0, 255);
  }
}

int btnPress(int lk, int rk) {
  int result = 0x00;
  if (lk == 0)
  {
    result += 0x80;
  }
  if (rk == 0)
  {
    result += 0x40;
  }
  return result;
}



