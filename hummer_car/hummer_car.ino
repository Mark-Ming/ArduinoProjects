#include <PS2X_lib.h>
#include <Servo.h>

#define MotorA_IN1 7
#define MotorA_IN2 6
#define MotorB_IN1 5
#define MotorB_IN2 4

#define PS2_DAT        8
#define PS2_CMD        9
#define PS2_SEL        10
#define PS2_CLK        11

#define pressures   false
#define rumble      false

PS2X ps2x;
Servo barrel; // 炮台舵机

int error = 0;
byte type = 0;
byte vibrate = 0;
int initialAngle = 90; //舵机初始角度
int max_Angle = 150; //最大角度
int min_Angle = 50; //最小角度
int per_Angle = 5; //每次角度变化量
int speed_range[3] = {85, 170, 255}; //三个前进档位速度上限
int reverse_max_speed = 85; //倒挡最大速度
int current_level = 0; //初始档位
unsigned long last_time = millis(); //上次档位变化的时间

int temp_x = 127; //记录上一次的x
int temp_y = 128; //记录上一次的y

void setup() {
  Serial.begin(9600);
  Serial.begin(115200);
  barrel.attach(22);
  barrel.write(initialAngle);
  pinMode(MotorA_IN1, OUTPUT);
  pinMode(MotorA_IN2, OUTPUT);
  pinMode(MotorB_IN1, OUTPUT);
  pinMode(MotorB_IN2, OUTPUT);
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  type = ps2x.readType();
}

void loop() {
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
    //    Serial.println(ps2x.Analog(PSS_LY), DEC);
    //
    //    Serial.println(initialAngle);


    //    Serial.print(ps2x.Analog(PSS_RY), DEC);
    //    Serial.print(",");
    //    Serial.println(ps2x.Analog(PSS_RX), DEC);

    if (ps2x.Button(PSB_L3))
      Serial.println("L3 pressed");
    if (ps2x.Button(PSB_R3))
      Serial.println("R3 pressed");
    if (ps2x.Button(PSB_L1))
    {
      Serial.println("L1 pressed");
    }
    if (ps2x.Button(PSB_R1))
    {
      Serial.println("R1 pressed");
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
      //加档
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
