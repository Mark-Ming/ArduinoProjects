#include <Servo.h>

//后马达 A(右) / B(左)
#define MotorA_IN1 7
#define MotorA_IN2 6
#define MotorB_IN1 5
#define MotorB_IN2 4

/* 马达相关变量 */
bool start_flag = false;              //卡车现在是否已经启动标识
bool stop_flag = true;                //卡车现在是否已经停止标识
int min_forward_value = 120;          //对应前进时最小速度(遥杆值)
int max_forward_value = 0;            //对应前进时最大速度(遥杆值)
int min_back_value = 150;             //对应后退时最小速度(遥杆值)
int max_back_value = 255;             //对应后退时最大速度(遥杆值)
int speed_range[3] = {120, 170, 255}; //三个前进档各自速度上限(遥杆值)
int back_speed_limit = 255;           //倒车最大速度[0-255]
int current_level = 2;                //档位 [初始为三档]
int current_speed = 120;              //前进速度(遥杆值) [ 120映射为0 ] => [120 ~ 0]
int turn_left_speed = 0;              //左转速度
int turn_right_speed = 0;             //右转速度


/* 舵机相关设置 */
Servo barrel1;                    //炮台舵机
int initialAngle = 90;            //炮台舵机初始角度
int max_Angle = 180;    //转向最大角度
int min_Angle = 0;     //转向最小角度
int per_Angle = 1;      //每次舵机角度变化量
static unsigned long last_back_change_timer = millis();      //上次升降状态变化的时间
static unsigned long last_back_change_step_timer = millis(); //每次变化时间间隔


/* 遥控部分 */
#define ROCKER_LEFT  16    //x向左
#define ROCKER_RIGHT 32    //x向右
#define ROCKER_UP    48    //y向上
#define ROCKER_DOWN  64    //y向下

#define ROCKER_ADVANCE 80   //y向上
#define ROCKER_RETREAT 96   //y向下
#define ROCKER_LEFT_2  128  //x向左
#define ROCKER_RETREAT 144  //x向右

#define BUTTON 112         // 按钮情况
#define ROTATE_TO_LEFT  4  //坦克炮台左旋转
#define ROTATE_TO_RIGHT 5  //坦克炮台右旋转
#define TANK_FIRE       6  //坦克开火
#define TANK_START      7  //坦克启动(播放声音)

#define HIGH_FOUR 240        //取出高四位key部分使用  按位与
#define LOW_FOUR  15         //取出低四位data部分使用  按位与

int key = 0;
int value = 0;
int value_button = 0;

int LX = 127; //左摇杆x初始值   控制左右转向
int RY = 127; //右摇杆y初始值   控制前进后退

unsigned char arr[4] = {};     // 遥控器信息原始数组

/*音频相关*/
static unsigned long music_timer = millis(); //每次变化时间间隔
bool clicked_fire_flag = false; //记录是否点击了开火
bool clicked_start_flag = false; //记录是否点击了启动

/*红外信号相关*/
bool attacked_flag = false; //记录是否被击中
static unsigned long  attacked_timer = millis(); //被击中坦克抖动的时间

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);              //遥控器使用
  Serial2.begin(9600);              //音频模块使用
  Serial.begin(115200);
  barrel1.attach(22);               //炮台舵机
  barrel1.write(initialAngle);
  volume(0x14);//音量设置0x00-0x1E
  pinMode(20, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);

  pinMode(3, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(20), attackedAtLeft, FALLING);
  attachInterrupt(digitalPinToInterrupt(21), attackedAtRight, FALLING);

  //设置模式开启
  //  pinMode(20, OUTPUT);
  //  analogWrite(20, LOW);
}

void loop() {
  //    Serial1.print("AT+RX\r\n");
  //    String str = Serial1.readString();
  //    Serial.println(str);
  Serial1.readBytes(arr, 4);       //读取遥控器信息
  handleData(arr);                 //处理遥控器信息

  //  Serial.print("LX : ");
  //  Serial.print(LX);
  //  Serial.print("RY : ");
  //  Serial.println(RY);
  //  Serial.print("按钮 : ");
  //  Serial.println(value_button);

  if (value_button == TANK_FIRE)
  {
    Serial.println("开火");
    clicked_fire_flag = true;
    music_timer = millis();
    fire();
    recoil();  // 后坐力
    volume(0x1E);
    play(0x02);
  }

  //开火声音播放完毕回复播放带速声音
  if (clicked_fire_flag == true && start_flag == true)
  {
    if (millis() - music_timer >= 5000 )
    {
      clicked_fire_flag = false;
      operationing();
    }
  }

  //开火声音播放完毕回复播放带速声音
  if (clicked_start_flag == true)
  {
    if (millis() - music_timer >= 11000)
    {
      clicked_start_flag = false;
      operationing();
    }
  }

  if (value_button == TANK_START)
  {
    if (!start_flag)
    {
      start_flag = true;   //启动带速声音
      clicked_start_flag = true;
      volume(0x14);
      play(0x03);
      music_timer = millis();
    }
    else
    {
      start_flag = false;  //关闭带速声音
      clicked_start_flag = false;
      clicked_fire_flag = false;
      stopMusic();
    }
  }

  if (value_button == ROTATE_TO_LEFT)
  {
    //    Serial.println("炮台左转");
    initialAngle += per_Angle;
    initialAngle = initialAngle >= max_Angle ? max_Angle : initialAngle;
    barrel1.write(initialAngle);
    Serial.print("当前角度:");
    Serial.println(initialAngle);
  }
  if (value_button == ROTATE_TO_RIGHT)
  {
    //    Serial.println("炮台右转");
    initialAngle -= per_Angle;
    initialAngle = initialAngle <= min_Angle ? min_Angle : initialAngle;
    barrel1.write(initialAngle);
  }

  if (RY > 120 && RY < 150)
  {
    trailStop();
  }
  if (RY <= 120)
  {
    advance(RY); //前进
  }
  else if (RY >= 150)
  {
    retreat(RY);//后退
  }
  delay(40);
}

//卡车前进函数 ---- 参数 : des_speed 当前遥杆传进来的速度
void advance(int des_speed)
{
  Serial.print("前进");
  motorForward(des_speed);
}

//卡车后退函数 ---- 参数 : des_speed 当前遥杆传进来的速度
void retreat(int des_speed)
{
  motorBackward(des_speed);
}

//卡车停止函数(缓慢停止)
void trailStop()
{
  //Motor_A
  analogWrite(MotorA_IN1, map(min_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));
  digitalWrite(MotorA_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(min_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));
}

//马达前转函数 ---- 参数 : des_speed 目标速度
void motorForward(int des_speed)
{
  des_speed = des_speed >= min_forward_value ? min_forward_value : des_speed;
  des_speed = des_speed <= max_forward_value ? max_forward_value : des_speed;
  turn_left_speed = 0;
  turn_right_speed = 0;
  //判断转向情况  LX
  if (LX <= 126 && LX >= 0)
  {
    //左转   左轮马达B减速
    turn_left_speed = map(LX, 126, 0, 0, 100);
  }
  else if (LX >= 128 && LX <= 255)
  {
    //右转   右轮马达A减速
    turn_right_speed = map(LX, 128, 255, 0, 100);
  }

  //Motor_A
  analogWrite(MotorA_IN1, map(des_speed + turn_left_speed, min_forward_value, max_forward_value, 0, speed_range[current_level]));
  digitalWrite(MotorA_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(des_speed + turn_right_speed , min_forward_value, max_forward_value, 0, speed_range[current_level]));
}

//马达后转函数 ---- 参数 : des_speed 目标速度
void motorBackward(int des_speed)
{
  des_speed = des_speed >= max_back_value ? max_back_value : des_speed;
  des_speed = des_speed <= min_back_value ? min_back_value : des_speed;
  //Motor_A
  digitalWrite(MotorA_IN1, LOW);
  analogWrite(MotorA_IN2, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  //Motor_B
  analogWrite(MotorB_IN1, map(des_speed, min_back_value, max_back_value, 0, back_speed_limit));
  digitalWrite(MotorB_IN2, LOW);
}

//处理接受到的数据包
void handleData(unsigned char *arr)
{
  LX = 127;
  RY = 127;
  value_button = 0;
  for (int i = 0; i < 4; i++)
  {
    key = arr[i] & HIGH_FOUR;
    value = arr[i] & LOW_FOUR;
    switch (key) {
      case ROCKER_LEFT:  LX = map(value, 15, 1, 128, 255); break;  //摇杆位置旋转了180度
      case ROCKER_RIGHT: LX = map(value, 1, 15, 126, 0); break;    //摇杆位置旋转了180度
      //      case ROCKER_UP:  LY = map(value, 15, 1, 126, 0); break;
      //      case ROCKER_DOWN: LY = map(value, 1, 15, 128, 255); break;
      case ROCKER_ADVANCE:  RY = map(value, 15, 1, 126, 0); break;
      case ROCKER_RETREAT: RY = map(value, 1, 15, 128, 255); break;
      case BUTTON: value_button = value; break;
    }
  }
}


//开火
void fire()
{
  digitalWrite(3, HIGH);
  delay(600);
  digitalWrite(3, LOW);
}

//后坐力
void recoil()
{
  motorBackward(max_back_value);
  delay(80);
  motorForward(max_forward_value);
  delay(80);
}

//左侧被击中的反应
void attackedAtLeft()
{
  Serial.println("左侧被击中");
  initialAngle = 80;
  barrel1.write(initialAngle);
  for (int i = 0; i < 2000; i++)
  {
    //    //Motor_A
    analogWrite(MotorA_IN1, map(max_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));
    digitalWrite(MotorA_IN2, LOW);

    //    //Motor_B
    analogWrite(MotorB_IN1, map(max_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));
    digitalWrite(MotorB_IN2, LOW);
  }
}

//右侧被击中的反应
void attackedAtRight()
{
  Serial.println("右侧被击中");
  initialAngle = 120;
  barrel1.write(initialAngle);
  for (int i = 0; i < 2000; i++)
  {

    //    //Motor_A
    digitalWrite(MotorA_IN1, LOW);
    analogWrite(MotorA_IN2, map(max_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));

    //    //Motor_B
    digitalWrite(MotorB_IN1, LOW);
    analogWrite(MotorB_IN2, map(max_forward_value, min_forward_value, max_forward_value, 0, speed_range[current_level]));

  }
}


/*音频相关*/
void play(unsigned char Track)
{
  unsigned char play[6] = {0xAA, 0x07, 0x02, 0x00, Track, Track + 0xB3}; //0xB3=0xAA+0x07+0x02+0x00,即最后一位为校验和
  Serial2.write(play, 6);
}
void volume( unsigned char vol)
{
  unsigned char volume[5] = {0xAA, 0x13, 0x01, vol, vol + 0xBE}; //0xBE=0xAA+0x13+0x01,即最后一位为校验和
  Serial2.write(volume, 5);
}
void stopMusic()
{
  unsigned char play[4] = {0xAA, 0x04, 0x00, 0xAE};
  Serial2.write(play, 4);
}
void operationing()  //播放坦克带速声音、复读
{
  volume(0x14);
  play(0x01);
  //复读指令
  unsigned char playLoop[8] = {0xAA, 0x20, 0x04, 0x00, 0x00, 0x00, 0x02, 0xAA + 0x20 + 0x04 + 0x02};
  Serial2.write(playLoop, 8);
}
