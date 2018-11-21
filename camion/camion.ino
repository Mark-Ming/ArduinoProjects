#include <PS2X_lib.h>
#include <Servo.h>
#include <CBMusic.h>

//后马达 A(右) / B(左)
#define MotorA_IN1 13
#define MotorA_IN2 12
#define MotorB_IN1 11
#define MotorB_IN2 10

//前马达 C(右) / D(左)
#define MotorC_IN1 5
#define MotorC_IN2 4
#define MotorD_IN1 3
#define MotorD_IN2 2

/* 手柄相关设置 */
#define PS2_DAT  6
#define PS2_CMD  7
#define PS2_SEL  8
#define PS2_CLK  9
PS2X ps2x;
int error = 0;
byte type = 0;
byte vibrate = 0;

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

/* 转向舵机相关设置 */
Servo barrel1;          //转向舵机(前)
Servo barrel2;          //转向舵机(后)
int initialAngle = 90;  //舵机初始角度
int max_Angle = 115;    //转向最大角度
int min_Angle = 65;     //转向最小角度
int per_Angle = 5;      //每次舵机角度变化量
int temp_x = 127;       //记录上一次的x
int temp_y = 128;       //记录上一次的y

/* 音频相关设置 */
DFRobotDFPlayerMini mp3;                              //MP3播放对象
static unsigned long music_timer = millis();          //带速音频播放时间间隔变量
static unsigned long start_trailer_timer = millis();  //启动按钮时间间隔变量
bool tape_speed = false;                              //是否可以带速标示

void setup() {
  Serial.begin(9600);
  Serial.begin(115200);
  Serial1.begin(9600); //Mp3使用
  barrel1.attach(22);
  barrel2.attach(23);
  barrel1.write(initialAngle);
  barrel2.write(initialAngle);

  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, false, false);
  type = ps2x.readType();

  mp3.begin(Serial1);
  mp3.volume(30);  //设置音量  0 - 30
  mp3.disableLoop(); //取消循环
}

void loop() {

    Serial.print("当前汽车状态:");
    Serial.println(start_flag);

  //带速状态
  if (start_flag && tape_speed)
  {
      mp3.loop(2);
      tape_speed = false;
  }

  //手柄情况
  if (type != 2)
  {
    ps2x.read_gamepad(false, vibrate);
    if (ps2x.Button(PSB_START)) //will be TRUE as long as button is pressed
      Serial.println("Start is being held");
    if (ps2x.Button(PSB_SELECT))
      Serial.println("Select is being held");
    if (ps2x.Button(PSB_PAD_UP)) {     //will be TRUE as long as button is pressed
      Serial.print("Up held this hard: ");
    }
    if (ps2x.Button(PSB_PAD_RIGHT)) {
      Serial.print("Right held this hard: ");
    }
    if (ps2x.Button(PSB_PAD_LEFT)) {
      Serial.print("LEFT held this hard: ");
    }
    if (ps2x.Button(PSB_PAD_DOWN)) {
      Serial.print("DOWN held this hard: ");
    }
    vibrate = ps2x.Analog(PSAB_CROSS);  //this will set the large motor vibrate speed based on how hard you press the blue (X) button

    //    Serial.print("Stick Values:");
    //    Serial.print(ps2x.Analog(PSS_LX), DEC);
    //    Serial.print(",");
    //    Serial.print(ps2x.Analog(PSS_LY), DEC);
    //    Serial.print(",");
    //    Serial.print(ps2x.Analog(PSS_RX), DEC);
    //    Serial.print(",");
    Serial.println(ps2x.Analog(PSS_RY), DEC);

    if (ps2x.Button(PSB_L1))
    {
      if (millis() - start_trailer_timer >= 3000)
      {
        start_trailer_timer = millis();
        Serial.println("L1 pressed");
        //启动汽车:
        if (!start_flag)
        {
          mp3.play(1);
          start_flag = true; //启动，设置为true
          tape_speed = true;
        }
        else
        {
          mp3.pause();
          start_flag = false; //熄火，设置为false
          tape_speed = false; //带速关闭
        }
      }
    }
    if (ps2x.Button(PSB_R1))
    {
      brake(); //刹车
    }
    if (ps2x.Button(PSB_L2))
    {
      Serial.println("L2 pressed");
    }
    if (ps2x.Button(PSB_R2))
    {
      Serial.println("R2 pressed");
    }
    if (ps2x.Button(PSB_CIRCLE))
    {
      Serial.println("Circle pressed");
    }
    if (ps2x.Button(PSB_SQUARE))
    {
      Serial.println("Square pressed");
    }
    if (ps2x.Button(PSB_TRIANGLE))
    {
      //加档
      if (millis() - last_level_timer > 2000)
      {
        last_level_timer = millis();
        current_level = current_level == 2 ? 2 : (++current_level);
      }
    }
    if (ps2x.Button(PSB_CROSS))
    {
      //减档
      if (millis() - last_level_timer > 2000)
      {
        last_level_timer = millis();
        current_level = current_level == 0 ? 0 : (--current_level);
      }
    }

    int LX = ps2x.Analog(PSS_LX);
    int LY = ps2x.Analog(PSS_LY);
    int RX = ps2x.Analog(PSS_RX);
    int RY = ps2x.Analog(PSS_RY);

    //方向盘自动归正
    if (LX == 128 && LY == 127 && initialAngle != 90)
    {
      initialAngle < 90 ? (initialAngle += per_Angle) : (initialAngle -= per_Angle);
      initialAngle = initialAngle == 90 ? 90 : initialAngle;
      barrel1.write(initialAngle);
      barrel2.write(initialAngle);
    }

    swerve(LX, LY);  // 舵机实时跟随遥杆转向

    if (RY > 120 && RY < 150)
    {
      trailStop();//停止
    }
    if (RY <= 120)
    {
      current_status = 1;
      if (start_flag)
      {
        advance(RY); //前进
      }
    }
    else if (RY >= 150)
    {
      current_status = -1;
      if (start_flag)
      {
        retreat(RY);//后退
      }

    }
  }
  delay(40);
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
      mp3.play(3);
      delay(20);
      mp3.loop(2);
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
      mp3.play(3);
      delay(20);
      mp3.loop(2);
      return;
    }
  }
}

//刹车函数
void brake()
{
  if (!stop_flag)
  {
    motorForward(min_forward_value);
    current_speed = min_forward_value;
    current_back_speed = min_back_value;
    current_status = 0;
    stop_flag = true;
    mp3.play(3);
  }
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
