#include <Servo.h>

// =================== 基本参数 ===================
const int TotalNumberofServos = 6;      // 6 个舵机

Servo myServos[TotalNumberofServos];

float pi = 3.14159;
float Shift;                            // 相邻关节之间的相位差
int InteriorAngle, SetpointAngle, MaxAngleDisplacement;

// 根据实际接线改这里：6 个舵机信号线对应的引脚
const int servoPins[TotalNumberofServos] = {
  2, 3, 4, 5, 6, 7
};

// 每个舵机的零点偏差（单位：度），之后可以按需要微调
int servoOffset[TotalNumberofServos] = {0, 0, 0, 0, 0, 0};

// =================== 工具函数 ===================

// 所有舵机设为同一个角度（不加偏差）
void setAllServos(int angle) {
  for (int i = 0; i < TotalNumberofServos; i++) {
    myServos[i].write(angle);
  }
}

// Raw Reset：全部打到 90°（机械中位，不加偏差）
void rawReset90() {
  setAllServos(90);
}

// Home 姿态：直线 + 软件偏差
void homePose() {
  for (int i = 0; i < TotalNumberofServos; i++) {
    int target = 90 + servoOffset[i];
    if (target < 0) target = 0;
    if (target > 180) target = 180;
    myServos[i].write(target);
  }
}

// 尺蠖用的小“拱”
void applyHump(int baseIndex, int pos) {
  const int humpSize = 3;
  const int pattern[humpSize] = { -1, 2, -1 };

  // 先拉直到 90°
  rawReset90();

  for (int k = 0; k < humpSize; k++) {
    int idx = baseIndex + k;
    if (idx >= 0 && idx < TotalNumberofServos) {
      int angle = 90 + pattern[k] * pos;
      if (angle < 0) angle = 0;
      if (angle > 180) angle = 180;
      myServos[idx].write(angle);
    }
  }
}

// =================== 各种动作 ===================

// 1. 直线（用 Home 姿态）
void straightline() {
  homePose();
}

// 2. C 形（全部往一侧偏）
void Cshape() {
  setAllServos(60);
}

// 3. U 形：中间几节强弯
void ubend() {
  int mid1, mid2;
  if (TotalNumberofServos % 2 == 0) {
    mid1 = TotalNumberofServos / 2 - 1;
    mid2 = TotalNumberofServos / 2;
  } else {
    mid1 = TotalNumberofServos / 2;
    mid2 = -1;
  }

  for (int i = 0; i < TotalNumberofServos; i++) {
    if (i == mid1 || i == mid2) {
      myServos[i].write(0);
    } else {
      myServos[i].write(90);
    }
    delay(50);
  }
}

// 4. 环形：弯成接近闭环的多边形
void ring() {
  InteriorAngle = 180 - 360 / (TotalNumberofServos + 1);
  SetpointAngle = abs(InteriorAngle - 90);
  for (int i = 0; i < TotalNumberofServos; i++) {
    myServos[i].write(SetpointAngle);
    delay(50);
  }
}

// 5. 蛇形爬行 slither
void slither(int offset, int Amplitude, int Speed, float Wavelengths) {
  MaxAngleDisplacement = abs(offset) + abs(Amplitude);

  // 确保不会超过 [0,180]
  while (MaxAngleDisplacement > 90) {
    Amplitude = abs(Amplitude) - 1;
    MaxAngleDisplacement = abs(offset) + Amplitude;
  }

  for (int i = 0; i < 360; i++) {
    float rads = i * pi / 180.0;

    for (int j = 0; j < TotalNumberofServos; j++) {
      float angle = 90 + offset +
                    Amplitude * sin(Speed * rads + j * Wavelengths * Shift);
      if (angle < 0) angle = 0;
      if (angle > 180) angle = 180;
      myServos[j].write((int)angle);
    }
    delay(10);
  }
}

// 6. 静态波形 staticWave
void staticWave(int offset, int Amplitude, float Wavelengths) {
  MaxAngleDisplacement = abs(offset) + abs(Amplitude);

  while (MaxAngleDisplacement > 90) {
    Amplitude = abs(Amplitude) - 1;
    MaxAngleDisplacement = abs(offset) + Amplitude;
  }

  for (int j = 0; j < TotalNumberofServos; j++) {
    float angle = 90 + offset +
                  Amplitude * sin(j * Wavelengths * Shift);
    if (angle < 0) angle = 0;
    if (angle > 180) angle = 180;
    myServos[j].write((int)angle);
    delay(15);
  }
}

// 7. 尺蠖步态 InchWorm
void InchWorm() {
  const int humpSize = 3;
  const int maxPos   = 30;

  for (int base = 0; base <= TotalNumberofServos - humpSize; base++) {
    for (int pos = 0; pos <= maxPos; pos++) {
      applyHump(base, pos);
      delay(10);
    }
  }

  homePose();
  delay(200);
}

// =================== 串口菜单相关 ===================

void printHelp() {
  Serial.println(F("===== Snake Robot 控制菜单 ====="));
  Serial.println(F("r: Raw Reset, 所有舵机到 90 度（无偏差）"));
  Serial.println(F("0: Home 复位到直线 (90+偏差)"));
  Serial.println(F("1: slither 直行"));
  Serial.println(F("2: slither 右转"));
  Serial.println(F("3: slither 左转"));
  Serial.println(F("4: staticWave 1.5 波长"));
  Serial.println(F("5: InchWorm 尺蠖步态"));
  Serial.println(F("6: U 形 (ubend)"));
  Serial.println(F("7: 圈形 (ring)"));
  Serial.println(F("8: C 形 (Cshape)"));
  Serial.println(F("9: 拉直 (straightline = Home)"));
  Serial.println(F("h: 再次打印帮助"));
  Serial.println(F("================================"));
}

// 根据输入字符执行动作
void handleKey(char c) {
  switch (c) {
    case 'r':
    case 'R':
      Serial.println(F("[动作] Raw Reset: 所有舵机到 90 度（无偏差）"));
      rawReset90();
      break;

    case '0':
      Serial.println(F("[动作] Home: 复位到直线 (带偏差)"));
      homePose();
      break;

    case '1':
      Serial.println(F("[动作] slither 直行"));
      slither(0, 35, 2, 1.5);
      break;

    case '2':
      Serial.println(F("[动作] slither 右转"));
      slither(10, 35, 2, 1.5);
      break;

    case '3':
      Serial.println(F("[动作] slither 左转"));
      slither(-10, 35, 2, 1.5);
      break;

    case '4':
      Serial.println(F("[动作] staticWave 1.5 波长"));
      staticWave(0, 35, 1.5);
      break;

    case '5':
      Serial.println(F("[动作] InchWorm 尺蠖步态"));
      InchWorm();
      break;

    case '6':
      Serial.println(F("[动作] U 形 ubend"));
      ubend();
      break;

    case '7':
      Serial.println(F("[动作] 圈形 ring"));
      ring();
      break;

    case '8':
      Serial.println(F("[动作] C 形 Cshape"));
      Cshape();
      break;

    case '9':
      Serial.println(F("[动作] 拉直 straightline (Home)"));
      straightline();
      break;

    case 'h':
    case 'H':
      printHelp();
      break;

    default:
      Serial.println(F("未知命令，输入 h 查看帮助"));
      break;
  }
}

// =================== Arduino 标准入口 ===================

void setup() {
  Serial.begin(115200);

  // 计算相位差
  Shift = 2 * pi / TotalNumberofServos;

  // 绑定舵机引脚
  for (int i = 0; i < TotalNumberofServos; i++) {
    myServos[i].attach(servoPins[i]);
  }

  // 上电后先复位到 Home 姿态
  homePose();
  delay(1000);

  printHelp();
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      return;  // 忽略换行
    }
    handleKey(c);
  }
}
