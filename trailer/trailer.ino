#include <PS2X_lib.h>
#include <Servo.h>
#include <CBMusic.h>

//后马达 A / B
#define MotorA_IN1 13
#define MotorA_IN2 12
#define MotorB_IN1 11
#define MotorB_IN2 10

//前马达 C / D
#define MotorC_IN1 5
#define MotorC_IN2 4
#define MotorD_IN1 3
#define MotorD_IN2 2

//手柄
#define PS2_DAT        6
#define PS2_CMD        7
#define PS2_SEL        8
#define PS2_CLK        9

#define pressures   false
#define rumble      false

PS2X ps2x;
Servo barrel1; //转向舵机(前)
Servo barrel2; //转向舵机(前)

int error = 0;
byte type = 0;
byte vibrate = 0;
int initialAngle = 90; //舵机初始角度
int max_Angle = 130; //最大角度
int min_Angle = 60; //最小角度
int per_Angle = 5 ; //每次角度变化量
int speed_range[3] = {85, 170, 255}; //三个前进档位速度上限
int current_speed = 120; // 当前速度《120映射为0》
int reverse_max_speed = 85; //倒挡最大速度
int current_level = 0; //初始档位
static unsigned long last_time = millis(); //上次档位变化的时间
static unsigned long change_speed_timer = millis();

int temp_x = 127; //记录上一次的x
int temp_y = 128; //记录上一次的y

//音频相关：
//MP3播放对象
DFRobotDFPlayerMini mp3;
//计算时间间隔变量
static unsigned long music_timer = 0;
//判断车现在是否已经启动
bool start_flag = false;
//判断车现在是否在停止
bool stop_flag = true;

void setup() {
  Serial.begin(9600);
  Serial.begin(115200);
  Serial1.begin(9600);
  barrel1.attach(22);
  barrel2.attach(23);
  barrel1.write(initialAngle);
  barrel2.write(initialAngle);

  pinMode(MotorA_IN1, OUTPUT);
  pinMode(MotorA_IN2, OUTPUT);
  pinMode(MotorB_IN1, OUTPUT);
  pinMode(MotorB_IN2, OUTPUT);

  pinMode(MotorC_IN1, OUTPUT);
  pinMode(MotorC_IN2, OUTPUT);
  pinMode(MotorD_IN1, OUTPUT);
  pinMode(MotorD_IN2, OUTPUT);
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  //  type = ps2x.readType();

  mp3.begin(Serial1);
  mp3.volume(30);  //设置音量  0 - 30
  mp3.disableLoop(); //取消循环
}

void loop() {
  //判断汽车是否已经启动
  if (start_flag)
  {
    if (millis() - music_timer >= 5000)
    {
      mp3.loop(2);
      music_timer = millis();
    }
  }

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
    //    Serial.print(ps2x.Analog(PSS_LX), DEC); //Left stick, Y axis. Other options: LX, RY, RX
    //    Serial.print(",");
    //    Serial.print(ps2x.Analog(PSS_LY), DEC);
    //    Serial.print(",");
    //    Serial.print(initialAngle);
    //    Serial.print(",");
    //    Serial.print(ps2x.Analog(PSS_RY), DEC);
    //    Serial.print(",");
    //    Serial.println(ps2x.Analog(PSS_RX), DEC);

    if (ps2x.Button(PSB_L3))
      Serial.println("L3 pressed");
    if (ps2x.Button(PSB_R3))
      Serial.println("R3 pressed");
    if (ps2x.Button(PSB_L1))
    {
      if (millis() - music_timer >= 2000)
      {
        Serial.println("L1 pressed");
        //启动汽车:
        if (!start_flag && stop_flag)
        {
          mp3.play(1);
          start_flag = true; //启动，设置为true
          stop_flag = false;
        }
        else
        {
          mp3.pause();
          start_flag = false; //熄火，设置为false
          stop_flag = true;
        }
        music_timer = millis();
      }
    }
    if (ps2x.Button(PSB_R1))
    {
      Serial.println("R1 pressed");
      //急刹车
      brakes();
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

    }
    if (ps2x.Button(PSB_SQUARE))
    {

    }
    if (ps2x.Button(PSB_TRIANGLE))
    {
      //加档
      if (millis() - last_time > 3000)
      {
        current_level = current_level == 2 ? 2 : (++current_level);
        Serial.print("减档，当前档位：");
        Serial.println(current_level);
        last_time = millis();
      }
      else {
        Serial.println("时间间隔过短不可连续加档");
      }
    }
    if (ps2x.Button(PSB_CROSS))
    {
      //减档
      if (millis() - last_time > 3000)
      {
        current_level = current_level == 0 ? 0 : (--current_level);
        Serial.print("减档，当前档位：");
        Serial.println(current_level);
        last_time = millis();
      }
      else {
        Serial.println("时间间隔过短不可连续减档");
      }
    }

    int LY = ps2x.Analog(PSS_LY);
    int LX = ps2x.Analog(PSS_LX);
    int RY = ps2x.Analog(PSS_RY);
    int RX = ps2x.Analog(PSS_RX);

    if (LX == 128 && LY == 127 && initialAngle != 90)
    {
      initialAngle < 90 ? (initialAngle += per_Angle) : (initialAngle -= per_Angle);
      initialAngle = initialAngle == 90 ? 90 : initialAngle;
      barrel1.write(initialAngle);
      barrel2.write(initialAngle);
    }

    swerve(LX, LY); // 实时转向

    if (RY > 120 && RY < 135)
    {
      tank_stop(); //停止
    }
    if (RY <= 120)
    {
      advance(RY); //前进
    }
    else if (RY >= 135)
    {
      retreat(RY); //后退
    }
  }
  delay(50);
}

//前进
void advance(int speed) {
  Serial.print("上一次速度:");
  Serial.println(current_speed);
  stop_flag = false;
  if (speed > current_speed) {
    //减速
    if (millis() - change_speed_timer > 200) {
      current_speed += 3;
      current_speed = current_speed >= 120 ? 120 : current_speed;
      Serial.print("减速，当前速度:");
      Serial.println(current_speed);
      //Motor_A
      analogWrite(MotorA_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorA_IN2, LOW);
      analogWrite(MotorC_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorC_IN2, LOW);
      //Motor_B
      digitalWrite(MotorB_IN1, LOW);
      analogWrite(MotorB_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorD_IN1, LOW);
      analogWrite(MotorD_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
    }
  }
  if (speed < current_speed) {
    //加速
    if (millis() - change_speed_timer > 200) {
      current_speed -= 3;
      current_speed = current_speed <= 0 ? 0 : current_speed;
      Serial.print("加速，当前速度:");
      Serial.println(current_speed);
      //Motor_A
      analogWrite(MotorA_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorA_IN2, LOW);
      analogWrite(MotorC_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorC_IN2, LOW);
      //Motor_B
      digitalWrite(MotorB_IN1, LOW);
      analogWrite(MotorB_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
      digitalWrite(MotorD_IN1, LOW);
      analogWrite(MotorD_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
    }
  }
}

//后退
void retreat(int speed) {
  stop_flag = false;
  //Motor_A
  analogWrite(MotorA_IN1, map(speed, 135, 255, 150, 80));
  digitalWrite(MotorA_IN2, HIGH);
  analogWrite(MotorC_IN1, map(speed, 135, 255, 150, 80));
  digitalWrite(MotorC_IN2, HIGH);
  //Motor_B
  digitalWrite(MotorB_IN1, HIGH);
  analogWrite(MotorB_IN2, map(speed, 135, 255, 150, 80));
  digitalWrite(MotorD_IN1, HIGH);
  analogWrite(MotorD_IN2, map(speed, 135, 255, 150, 80));
}

//减速缓慢停止
void tank_stop() {
  if (millis() - change_speed_timer > 200 && current_speed != 120)
  {
    current_speed += 5;
    current_speed = current_speed >= 120 ? 120 : current_speed;
    //Motor_A
    analogWrite(MotorA_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
    digitalWrite(MotorA_IN2, LOW);
    analogWrite(MotorC_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
    digitalWrite(MotorC_IN2, LOW);
    //Motor_B
    digitalWrite(MotorB_IN1, LOW);
    analogWrite(MotorB_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
    digitalWrite(MotorD_IN1, LOW);
    analogWrite(MotorD_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
    if (!stop_flag && start_flag && current_speed == 120)
    {
      mp3.play(3);
      stop_flag = true;
    }
  }

}

//急停
void brakes()
{
  if (!stop_flag && start_flag)
  {
    mp3.play(3);
    stop_flag = true;
  }
  current_speed = 120;
  //Motor_A
  analogWrite(MotorA_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
  digitalWrite(MotorA_IN2, LOW);
  analogWrite(MotorC_IN1, map(current_speed, 120, 0, 0, speed_range[current_level]));
  digitalWrite(MotorC_IN2, LOW);
  //Motor_B
  digitalWrite(MotorB_IN1, LOW);
  analogWrite(MotorB_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));
  digitalWrite(MotorD_IN1, LOW);
  analogWrite(MotorD_IN2, map(current_speed, 120, 0, 0, speed_range[current_level]));

  //  //Motor_A
  //  analogWrite(MotorA_IN1, 0);
  //  digitalWrite(MotorA_IN2, LOW);
  //  //Motor_C
  //  analogWrite(MotorC_IN1, 0);
  //  digitalWrite(MotorC_IN2, LOW);
  //  //Motor_B
  //  digitalWrite(MotorB_IN1, LOW);
  //  analogWrite(MotorB_IN2, 0);
  //  //Motor_D
  //  digitalWrite(MotorD_IN1, LOW);
  //  analogWrite(MotorD_IN2, 0);
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
