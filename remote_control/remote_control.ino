
/* 左侧遥杆 */
#define ROCKER_LEFT  16  //x向左
#define ROCKER_RIGHT 32  //x向右
#define ROCKER_UP    48  //y向上
#define ROCKER_DOWN  64  //y向下

/* 右侧遥杆 */
#define ROCKER_ADVANCE 80 //y向上
#define ROCKER_RETREAT 96 //y向下

#define BUTTON 112  //除了遥杆的按钮情况

unsigned char arr[4] = {};
int key = 0;
int x_direction = 0;   // 转向遥杆x轴方位  0 静止、-1 左、1 右
int y_direction = 0;   // 转向遥杆y轴方位  0 静止、-1 上、1 下
int r_y_direction = 0; // 前进遥杆y轴方位  0 静止、-1 上、1 下

int LX = 0;     //方向遥杆x值
int LY = 0;     //方向遥杆y值
int RY = 0;     //前进遥杆y值

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  //设置模式开启
  pinMode(20, OUTPUT);
  analogWrite(20, LOW);

//  Serial1.print("AT+C220\r\n");
//  Serial1.print("AT+RX\r\n");
//  String str = Serial1.readString();
//  Serial.println(str);
}

void loop() {

//  Serial1.print("AT+C230\r\n");
  Serial1.print("AT+RX\r\n");
  String str = Serial1.readString();
  Serial.println(str);

  
  //  LX = getRockerValue(A0);  // 转向遥杆x轴
  //  LY = getRockerValue(A1);  // 转向遥杆y轴
  //  RY = getRockerValue(A13); // 前进遥杆y轴

  //  LX = analogRead(A0);   // 转向遥杆x轴
  //  LY = analogRead(A1);   // 转向遥杆y轴
  //  RY = analogRead(A14);  // 前进遥杆y轴
  //
  //  x_direction = judgeDirection(LX);
  //  y_direction = judgeDirection(LY);
  //  r_y_direction = judgeDirection(RY);

  //  switch (x_direction) {
  //    case -1: key = ROCKER_LEFT; break;
  //    case  0: key = 0; break;
  //    case  1: key = ROCKER_RIGHT; break;
  //  }
  //将x信息存入arr[0]
  //  arr[0] = key | getRockerValue(LX);  // 高四位为key,低四位为value

  //  switch (y_direction) {
  //    case -1: key = ROCKER_UP; break;
  //    case  0: key = 0; break;
  //    case  1: key = ROCKER_DOWN; break;
  //  }
  //将y信息存入arr[1]
  //  arr[1] = key | getRockerValue(LY);  // 高四位为key,低四位为value

  //  switch (r_y_direction) {
  //    case -1: key = ROCKER_ADVANCE; break;
  //    case  0: key = 0; break;
  //    case  1: key = ROCKER_RETREAT; break;
  //  }
  //将ry信息存入arr[2]
  //  arr[2] = key | getRockerValue(RY);  // 高四位为key,低四位为value

  //将按钮信息存入arr[3]   key == button
  //  key = BUTTON;
  //  arr[3] = key | 1;  // 高四位为key,低四位为value

  //  Serial.print("   LX:");
  //  Serial.print(LX);
  //  Serial.print("   LY:");
  //  Serial.print(LY);
  //  Serial.print(" ---- ");
  //  Serial.print("   RY:");
  //  Serial.println(RY);

  //发送数据
  //  Serial1.write(arr, 4);
  delay(50);
}

//整理遥杆数值
int getRockerValue(int value)
{
  value = value < 40 ? 40 : value;
  value = value > 990 ? 990 : value;
  if (value > 450 && value < 550)
  {
    return 0;
  }
  else if (value <= 450)    // x,y前半部分
  {
    return map(value, 450, 40, 15, 1);
  }
  else if (value >= 550)    // x,y后半部分
  {
    return map(value, 550, 990, 1, 15);
  }
}

//判断某个端口号对应轴的方向
int judgeDirection(int value)
{
  value = value < 40 ? 40 : value;
  value = value > 990 ? 990 : value;
  if (value <= 450)
  {
    return -1;
  }
  else if (value > 450 && value < 550)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}
