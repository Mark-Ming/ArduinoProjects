#include <Servo.h>

#define MotorA_IN1 7
#define MotorA_IN2 6
#define MotorB_IN1 5
#define MotorB_IN2 4

Servo barrel; // 炮台舵机
int initialAngle = 90; //舵机初始角度
int max_Angle = 150; //最大角度
int min_Angle = 50; //最小角度
int per_Angle = 5; //每次角度变化量
int speed_range[3] = {85, 170, 255}; //三个前进档位速度上限
int reverse_max_speed = 85; //倒挡最大速度
int current_level = 0; //初始档位
unsigned long last_time = millis(); //上次档位变化的时间

int temp_x = 127; //记录上一次的x
int temp_y = 127; //记录上一次的y

#define ROCKER_LEFT  16
#define ROCKER_RIGHT 32
#define ROCKER_UP    48
#define ROCKER_DOWN  64

#define ROCKER_ADVANCE 80
#define ROCKER_RETREAT 96

#define BUTTON 112  //除了遥杆的按钮情况

#define HIGH_FOUR 240  //取出高四位key部分使用  按位与
#define LOW_FOUR  15   //取出低四位data部分使用  按位与

int key_x = 0;
int key_y = 0;
int key_r_y = 0;
int key_button = 0;

int value_x = 0;
int value_y = 0;
int value_r_y = 0;
int value_button = 0;

int LX = 127; //初始值
int LY = 127; //初始值
int RY = 127; //初始值

unsigned char arr[4] = {};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  Serial.begin(115200);
  barrel.attach(22);
  barrel.write(initialAngle);
  pinMode(MotorA_IN1, OUTPUT);
  pinMode(MotorA_IN2, OUTPUT);
  pinMode(MotorB_IN1, OUTPUT);
  pinMode(MotorB_IN2, OUTPUT);
}

void loop() {
  Serial1.readBytes(arr, 4);
  handleData(arr);

  Serial.print("x:");
  Serial.print(LX);
  Serial.print("  y:");
  Serial.print(LY);
  Serial.print("  ry:");
  Serial.println(RY);

  if (LX == 127 && LY == 127 && initialAngle != 90)
  {
    initialAngle < 90 ? (initialAngle += per_Angle) : (initialAngle -= per_Angle);
    barrel.write(initialAngle);
  }

  swerve(LX, LY);

  if (RY == 127)
  {
    tank_stop();
  }
  if (RY < 127)
  {
    advance(RY);
  }
  else if (RY > 127)
  {
    retreat(RY);
  }
  delay(50);
}

//前进
void advance(int speed) {
  //Motor_A
  analogWrite(MotorA_IN1, map(speed, 126, 0, 0, speed_range[current_level]));
  digitalWrite(MotorA_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(speed, 126, 0, 0, speed_range[current_level]));
}

//后退
void retreat(int speed) {
  //Motor_A
  analogWrite(MotorA_IN1, map(speed, 128, 255, 150, 80));
  digitalWrite(MotorA_IN2, HIGH);
  //Motor_B
  digitalWrite(MotorB_IN1, HIGH);
  analogWrite(MotorB_IN2, map(speed, 128, 255, 150, 80));
}

//左转
void moveLeft(int speed) {
  //Motor_A
  analogWrite(MotorA_IN1, map(speed, 128, 0, 255, 0));
  digitalWrite(MotorA_IN2, HIGH);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(speed, 128, 0, 0, 255));
}

//右转
void moveRight(int speed) {
  //Motor_A
  analogWrite(MotorA_IN1, map(speed, 0, 128, 0, 255));
  digitalWrite(MotorA_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, HIGH);
  analogWrite(MotorB_IN2, map(speed, 0, 128, 255, 0));
}

//停止
void tank_stop() {
  //Motor_A
  analogWrite(MotorA_IN1, 0);
  digitalWrite(MotorA_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, 0);
}

//计算转向
void swerve(int x, int y)
{
  //左下方
  if (x == 0 || y == 255)
  {
    if (temp_x == x && temp_y == y)
    {
      return;
    }
    if (x == 0)
    {
      initialAngle += (judgeBottomLeft(temp_y, y, 0) ? per_Angle : -per_Angle);
    }
    if (y == 0)
    {
      initialAngle += (judgeBottomLeft(temp_x, x, 0) ? per_Angle : -per_Angle);
    }
    initialAngle = initialAngle >= max_Angle ? max_Angle : initialAngle;
    initialAngle = initialAngle <= min_Angle ? min_Angle : initialAngle;
    barrel.write(initialAngle);
  }

  if (y == 0 || x == 255)
  {
    if (temp_x == x && temp_y == y)
    {
      return;
    }
    if (y == 0)
    {
      initialAngle += (judgeBottomLeft(temp_x, x, 1) ? per_Angle : -per_Angle);
    }
    if (x == 255)
    {
      initialAngle += (judgeBottomLeft(temp_y, y, 1) ? per_Angle : -per_Angle);
    }
    initialAngle = initialAngle >= max_Angle ? max_Angle : initialAngle;
    initialAngle = initialAngle <= min_Angle ? min_Angle : initialAngle;
    barrel.write(initialAngle);
  }
  temp_x = x;
  temp_y = y;
}

//判断增减性确定方向 (derection 左下方区域使用 0, 右上方区域使用 1)
bool judgeBottomLeft(int tempValue, int newValue, int derection)
{
  if (derection == 0)
  {
    return newValue >  tempValue ;
  }
  else
  {
    return !(newValue >  tempValue);
  }
}

//处理接受到的数据包
void handleData(unsigned char *arr)
{
  key_x = arr[0] & HIGH_FOUR;
  value_x = arr[0] & LOW_FOUR;
  switch (key_x) {
    case ROCKER_LEFT:  LX = map(value_x, 15, 1, 126, 0); break;
    case ROCKER_RIGHT: LX = map(value_x, 1, 15, 128, 255); break;
    case 0: LX = 127; break;
  }
  key_y = arr[1] & HIGH_FOUR;
  value_y = arr[1] & LOW_FOUR;
  switch (key_y) {
    case ROCKER_UP:  LY = map(value_y, 15, 1, 126, 0); break;
    case ROCKER_DOWN: LY = map(value_y, 1, 15, 128, 255); break;
    case 0: LY = 127; break;
  }
  key_r_y = arr[2] & HIGH_FOUR;
  value_r_y = arr[2] & LOW_FOUR;
  switch (key_r_y) {
    case ROCKER_ADVANCE:  RY = map(value_r_y, 15, 1, 126, 0); break;
    case ROCKER_RETREAT: RY = map(value_r_y, 1, 15, 128, 255); break;
    case 0: RY = 127; break;
  }

  //  key_button = arr[3] & HIGH_FOUR;
  //  value_button = arr[3] & LOW_FOUR;
}
