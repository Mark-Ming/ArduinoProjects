#include <Servo.h>

//后马达 A(右) / B(左)
#define MotorA_IN1 7
#define MotorA_IN2 6
#define MotorB_IN1 5
#define MotorB_IN2 4

//前马达 C(右) / D(左)
#define MotorC_IN1 13
#define MotorC_IN2 12
#define MotorD_IN1 11
#define MotorD_IN2 10

/* 马达相关变量 */
bool start_flag = false;              //卡车现在是否已经启动标识
bool stop_flag = true;                //卡车现在是否已经停止标识
int min_forward_value = 120;          //对应前进时最小速度(遥杆值)
int max_forward_value = 0;            //对应前进时最大速度(遥杆值)
int min_back_value = 150;             //对应后退时最小速度(遥杆值)
int max_back_value = 255;             //对应后退时最大速度(遥杆值)
int speed_range[3] = {120, 170, 255};  //三个前进档各自速度上限(遥杆值)
int back_speed_limit = 255;           //倒车最大速度[0-255]
int current_level = 0;                //档位 [初始为一档]
int current_speed = 120;              //前进速度(遥杆值) [120映射为0] => [120 ~ 0]
int current_back_speed = 140;         //后退速度(遥杆值) [140映射为0] => [140 ~ 255]
int per_speed = 5;                    //每次加速、减速的变化量
int current_status = 0;               //卡车当前状态  0:停止 1:前进 -1:后退
static unsigned long last_speed_timer = millis(); //上次速度变化的时间
static unsigned long last_level_timer = millis(); //上次档位变化的时间

/* 舵机相关设置 */
Servo barrel1;          //转向舵机(前)
Servo barrel2;          //舵机(后斗)
int initialAngle = 90;  //转向舵机初始角度
int back_initialAngle = 180; //后斗舵机初始角度
bool current_back_status = false; //当前后斗的状态，默认降下
int max_Angle = 120;    //转向最大角度
int min_Angle = 50;     //转向最小角度
int per_Angle = 5;      //每次舵机角度变化量
int temp_x = 127;       //记录上一次的x
int temp_y = 127;       //记录上一次的y
static unsigned long last_back_change_timer = millis();      //上次升降状态变化的时间
static unsigned long last_back_change_step_timer = millis(); //每次变化时间间隔


/* 遥控部分 */
#define ROCKER_LEFT  16
#define ROCKER_RIGHT 32
#define ROCKER_UP    48
#define ROCKER_DOWN  64

#define ROCKER_ADVANCE 80
#define ROCKER_RETREAT 96

#define BUTTON 112     // 按钮情况
#define ADD_LEVEL 1       //加档按钮
#define MINUS_LEVEL 2     //减档按钮
#define RISE_FALL 3       //后斗升降命令

#define HIGH_FOUR 240  //取出高四位key部分使用  按位与
#define LOW_FOUR  15   //取出低四位data部分使用  按位与

int key = 0;
int key_x = 0;
int key_y = 0;
int key_r_y = 0;
int key_button = 0;

int value = 0;
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
  barrel1.attach(28);    //前转向
  barrel2.attach(29);  //后斗
  barrel1.write(initialAngle);
  barrel2.write(back_initialAngle);
}

void loop() {

  //  advance(0);
  Serial1.readBytes(arr, 4);
  handleData(arr);

  //  Serial.print(arr[0]);
  //  Serial.print("    ");
  //  Serial.print(arr[1]);
  //  Serial.print("    ");
  //  Serial.print(arr[2]);
  //  Serial.print("    ");
  //  Serial.print(arr[3]);
  //  Serial.println("    ");

  Serial.print("LX : ");
  Serial.print(LX);
  Serial.print("LY : ");
  Serial.print(LY);
  Serial.print("RY : ");
  Serial.print(RY);
  Serial.print("按钮 : ");
  Serial.println(value_button);

  if (value_button == RISE_FALL)
  {
    if (millis() - last_back_change_timer > 2000)
    {
      if (!current_back_status) //升起
      {
        barrel2.write(120);
        current_back_status = true;
      }
      else //下降
      {
        barrel2.write(180);
        current_back_status = false;
      }
    }

  }

  if (value_button == ADD_LEVEL)
  {
    Serial.println("加档");
    //加档
    if (millis() - last_level_timer > 2000)
    {
      last_level_timer = millis();
      current_level = current_level == 2 ? 2 : (++current_level);
    }
  }
  if (value_button == MINUS_LEVEL)
  {
    Serial.println("减档");
    //减档
    if (millis() - last_level_timer > 2000)
    {
      last_level_timer = millis();
      current_level = current_level == 0 ? 0 : (--current_level);
    }
  }

  //方向盘自动归正
  if (LX == 127 && LY == 127 && initialAngle != 90)
  {
    initialAngle < 90 ? (initialAngle += per_Angle) : (initialAngle -= per_Angle);
    initialAngle = initialAngle == 90 ? 90 : initialAngle;
    barrel1.write(initialAngle);
    barrel2.write(initialAngle);
  }

  swerve(LX, LY);  // 舵机实时跟随遥杆转向

  if (RY > 120 && RY < 150)
  {
    brake();
  }
  if (RY <= 120)
  {
    advance(RY); //前进
  }
  else if (RY >= 150)
  {
    retreat(RY);//后退
  }
  delay(60);
}

//卡车前进函数 ---- 参数 : des_speed 当前遥杆传进来的速度
void advance(int des_speed)
{
  stop_flag = false;
  if (current_speed >= des_speed && (millis() - last_speed_timer > 150))
  {
    last_speed_timer = millis();
    Serial.print("开始加速:");
    current_speed -= per_speed;
    current_speed = current_speed <= des_speed ? des_speed : current_speed;
    motorForward(current_speed);
  }
  if (current_speed < des_speed && (millis() - last_speed_timer > 150))
  {
    last_speed_timer = millis();
    Serial.print("开始减速:");
    current_speed += per_speed;
    current_speed = current_speed >= des_speed ? des_speed : current_speed;
    motorForward(current_speed);
  }
}

//卡车后退函数 ---- 参数 : des_speed 当前遥杆传进来的速度
void retreat(int des_speed)
{
  stop_flag = false;
  if (current_back_speed < des_speed && (millis() - last_speed_timer > 150))
  {
    last_speed_timer = millis();
    Serial.print("开始加速后退:");
    current_back_speed += per_speed;
    current_back_speed = current_back_speed >= des_speed ? des_speed : current_back_speed;
    motorBackward(current_back_speed);
  }
  if (current_back_speed > des_speed && (millis() - last_speed_timer > 150))
  {
    last_speed_timer = millis();
    Serial.print("开始减速后退:");
    current_back_speed -= per_speed;
    current_back_speed = current_back_speed <= des_speed ? des_speed : current_back_speed;
    motorBackward(current_back_speed);
  }
}

//卡车停止函数(缓慢停止)
void trailStop()
{
  if (current_status == 1 && (millis() - last_speed_timer > 150) && current_speed != min_forward_value) //由前进开始减速到120
  {
    last_speed_timer = millis();
    Serial.print("减速停止中:");
    current_speed += per_speed;
    current_speed = current_speed >= min_forward_value ? min_forward_value : current_speed;
    motorForward(current_speed);
    if (current_speed == min_forward_value && !stop_flag)
    {
      current_status = 0;
      stop_flag = true;
      return;
    }
  }
  if (current_status == -1 && (millis() - last_speed_timer > 150) && current_back_speed != min_back_value) //由后退开始减速到150
  {
    last_speed_timer = millis();
    Serial.print("减速停止中:");
    current_back_speed -= per_speed;
    current_back_speed = current_back_speed <= min_back_value ? min_back_value : current_back_speed;
    motorBackward(current_back_speed);
    if (current_back_speed == min_back_value  && !stop_flag)
    {
      current_status = 0;
      stop_flag = true;
      return;
    }
  }
}

//刹车函数
void brake()
{
  motorForward(min_forward_value);
  current_speed = min_forward_value;
  current_back_speed = min_back_value;
  current_status = 0;
  stop_flag = true;
}

//马达前转函数 ---- 参数 : des_speed 目标速度
void motorForward(int des_speed)
{
  des_speed = des_speed >= min_forward_value ? min_forward_value : des_speed;
  des_speed = des_speed <= max_forward_value ? max_forward_value : des_speed;
  int left_differential = 0; //左转差速
  int right_differential = 0; //右转差速
  //  if (initialAngle <= 70)
  //  {
  //    left_differential = 15;
  //  }
  //  if (initialAngle >= 110)
  //  {
  //    right_differential = 15;
  //  }
  //Motor_A_C
  analogWrite(MotorA_IN1, map(des_speed - left_differential + right_differential, min_forward_value, max_forward_value, 0, speed_range[current_level]));
  digitalWrite(MotorA_IN2, LOW);
  analogWrite(MotorC_IN1, map(des_speed - left_differential + right_differential, min_forward_value, max_forward_value, 0, speed_range[current_level]));
  digitalWrite(MotorC_IN2, LOW);
  //Motor_B_D
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(des_speed - right_differential + left_differential, min_forward_value, max_forward_value, 0, speed_range[current_level]));
  digitalWrite(MotorD_IN1, LOW);
  analogWrite(MotorD_IN2, map(des_speed - right_differential + left_differential, min_forward_value, max_forward_value, 0, speed_range[current_level]));
}

//马达后转函数 ---- 参数 : des_speed 目标速度
void motorBackward(int des_speed)
{
  des_speed = des_speed >= max_back_value ? max_back_value : des_speed;
  des_speed = des_speed <= min_back_value ? min_back_value : des_speed;
  //Motor_A_C
  digitalWrite(MotorA_IN1, LOW);
  analogWrite(MotorA_IN2, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  digitalWrite(MotorC_IN1, LOW);
  analogWrite(MotorC_IN2, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  //Motor_B_D
  analogWrite(MotorB_IN1, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  digitalWrite(MotorB_IN2, LOW);
  analogWrite(MotorD_IN1, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  digitalWrite(MotorD_IN2, LOW);
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
    barrel1.write(initialAngle);
    barrel2.write(initialAngle);
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
    barrel1.write(initialAngle);
    barrel2.write(initialAngle);
  }
  temp_x = x;
  temp_y = y;
}

//判断增减性确定方向 (derection 左下方区域使用 0, 右上方区域使用 1)
bool judgeBottomLeft(int tempValue, int newValue, int derection)
{
  if (derection == 1)
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
  LX = 127;
  LY = 127;
  RY = 127;
  for (int i = 0; i < 4; i++)
  {
    key = arr[i] & HIGH_FOUR;
    value = arr[i] & LOW_FOUR;
    switch (key) {
      case ROCKER_LEFT:  LX = map(value, 15, 1, 126, 0); break;
      case ROCKER_RIGHT: LX = map(value, 1, 15, 128, 255); break;
      case ROCKER_UP:  LY = map(value, 15, 1, 126, 0); break;
      case ROCKER_DOWN: LY = map(value, 1, 15, 128, 255); break;
      case ROCKER_ADVANCE:  RY = map(value, 15, 1, 126, 0); break;
      case ROCKER_RETREAT: RY = map(value, 1, 15, 128, 255); break;
      case BUTTON: value_button = value; break;
    }
    //    key_x = arr[0] & HIGH_FOUR;
    //    value_x = arr[0] & LOW_FOUR;
    //    switch (key_x) {
    //      case ROCKER_LEFT:  LX = map(value_x, 15, 1, 126, 0); break;
    //      case ROCKER_RIGHT: LX = map(value_x, 1, 15, 128, 255); break;
    //      case 0: LX = 127; break;
    //    }
    //    key_y = arr[1] & HIGH_FOUR;
    //    value_y = arr[1] & LOW_FOUR;
    //    switch (key_y) {
    //      case ROCKER_UP:  LY = map(value_y, 15, 1, 126, 0); break;
    //      case ROCKER_DOWN: LY = map(value_y, 1, 15, 128, 255); break;
    //      case 0: LY = 127; break;
    //    }
    //    key_r_y = arr[2] & HIGH_FOUR;
    //    value_r_y = arr[2] & LOW_FOUR;
    //    switch (key_r_y) {
    //      case ROCKER_ADVANCE:  RY = map(value_r_y, 15, 1, 126, 0); break;
    //      case ROCKER_RETREAT: RY = map(value_r_y, 1, 15, 128, 255); break;
    //      case 0: RY = 127; break;
    //    }

    //按钮的值
    //  key_button = arr[3] & HIGH_FOUR;
    //  value_button = arr[3] & LOW_FOUR;
  }
}
