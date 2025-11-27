6-DOF Snake Robot 主程序 README
本程序用于控制一条由 6 个舵机组成的蛇形机器人，通过 Arduino 串口输入不同按键，触发不同动作（直行爬行、转弯、尺蠖、U 形、环形等），并带有「复位到 90°」和「直线 home 姿态」功能，方便调试与教学。

1. 环境与依赖
1.1 硬件要求
Arduino 主控板
推荐：Arduino Uno / Arduino Nano（ATmega328P）
舵机（Servo）× 6
建议：9g 舵机，如 SG90（塑料齿）、MG90S（金属齿）等
外部 5V 舵机电源（非常重要）
例如：5V 3A DC 适配器 / 降压模块
若干杜邦线、USB 线（Type-A → Micro-USB / Mini-USB / USB-B，根据你的板子）
注意：请不要用 Arduino 板载的 5V 引脚直接给 6 个舵机供电，会导致板子复位甚至损坏。
1.2 软件要求
Arduino IDE（建议 1.8.x 或 2.x 版本）
串口监视器（Arduino IDE 自带）
1.3 所需库
本程序只需要 Servo 库（Arduino 官方自带）：
#include <Servo.h>
一般情况下，Arduino IDE 已经自带此库，无需另装。如果编译时报错找不到 Servo：
在菜单栏选择：Sketch → Include Library → Manage Libraries...
搜索 Servo，安装官方的 Servo 库即可。

2. 硬件接线说明
2.1 舵机与 Arduino 引脚对应关系
程序中默认使用 6 个数字引脚：
const int TotalNumberofServos = 6;
const int servoPins[TotalNumberofServos] = {2, 3, 4, 5, 6, 7};
推荐约定如下（你也可以在 README 里画图）：
舵机编号
位置建议（可自定义）
Arduino 引脚
Servo 0
尾部关节
D2
Servo 1
第 2 节
D3
Servo 2
第 3 节
D4
Servo 3
第 4 节
D5
Servo 4
第 5 节
D6
Servo 5
头部关节
D7
常见舵机线序：
棕/黑：GND（地）
红：5V（电源正）
黄/橙/白：信号（PWM）
2.2 电源接法（重点）
推荐使用独立 5V 电源给舵机供电：
舵机电源：
6 个舵机的 红线 → 外部 5V 电源的 +
6 个舵机的 棕/黑线 → 外部 5V 电源的 -
Arduino 与舵机电源共地：
将外部 5V 电源的 - → 接到 Arduino 的 GND 引脚
这样 Arduino 和舵机之间就共地了，信号才可靠。
舵机信号：
6 个舵机的 信号线 分别接：D2, D3, D4, D5, D6, D7
Arduino 供电：
Arduino 通过 USB 连接电脑供电 (Mac需使用拓展坞)。
总结：
舵机电流由外部 5V 电源提供；
Arduino 只提供逻辑信号；
一定要共地：电源 GND ↔ Arduino GND ↔ 所有舵机 GND。

3. 程序使用方法
3.1 烧录程序
打开 Arduino IDE
将主程序 SnakeControl.ino 文件打开
在 Tools 菜单中选择对应的板子和端口：
Board: Arduino Uno 或 Arduino Nano (ATmega328P, Old Bootloader) 等
Port: 对应你的 USB 口
点击“上传”（右箭头按钮）。
3.2 打开串口监视器
在 Arduino IDE 中点击右上角“串口监视器”图标；
设置：
波特率：115200
行结束符：可选 “None” 或 “Both NL & CR”（程序会忽略回车换行）
上电初始化后，串口会自动打印出一个菜单（帮助信息）。
3.3 串口按键与动作对应表
在串口监视器中输入 单个字符（不需要回车也可以），即可触发对应动作。
按键（串口输入）
对应动作 / 函数
r / R
rawReset90()：Raw Reset：所有舵机转到 90°，不加偏差（用于确认机械零位）
0
homePose()：根据 servoOffset 把蛇恢复到“直线姿态”
1
slither(0, 35, 2, 1.5)：直行蛇形爬行
2
slither(10, 35, 2, 1.5)：带右偏的蛇形爬行（右转趋势）
3
slither(-10, 35, 2, 1.5)：带左偏的蛇形爬行（左转趋势）
4
staticWave(0, 35, 1.5)：静态波形（1.5 个波长）
5
InchWorm()：尺蠖步态，从尾到头依次“拱起”
6
ubend()：U 形，机器人身体中间强弯
7
ring()：环形，近似绕成一个圈
8
Cshape()：C 形，整体向一侧弯曲
9
straightline()：拉直（内部调用 homePose()）
h / H
再次打印帮助菜单
推荐调试流程：
上电后，先按 r → 看所有关节能否都转到大致的中位；
再按 0 或 9 → 看整体是否是你想要的直线 home 姿态。

4. 代码结构与逻辑说明
4.1 全局配置与状态
主要全局变量：
const int TotalNumberofServos = 6;      // 舵机数量
Servo myServos[TotalNumberofServos];    // 舵机对象数组float pi = 3.14...相邻舵机之间的相位差（蛇形波）int servoPins[6] = {2, 3, 4, 5, 6, 7};  // 舵机信号引脚
// 每个舵机的零点偏差（软件校正用）int servoOffset[TotalNumberofServos] = {0, 0, 0, 0, 0, 0};
Shift 用于 slither() 和 staticWave() 函数，控制每个关节之间的相位差，使整条蛇形成连续波。
servoOffset[] 用来做软件层零点校正：机械上装好后，如果某一节总是有点偏，可以在这里微调。
4.2 工具函数
setAllServos(int angle)
不加任何偏差，直接把所有舵机打到同一个角度。
主要用于 Raw Reset、内部初始化等。
rawReset90()
调用 setAllServos(90)；
所有关节转到 90°，方便你观察机械零位。
homePose()
把每个舵机设置为 90 + servoOffset[i]；
这是你定义的“视觉直线姿态”，用来作为蛇的基准姿势。
applyHump(int baseIndex, int pos)
尺蠖步态内部用的“小拱形”生成函数；
以 baseIndex 为起点，在 3 个相邻舵机上形成 -1, 2, -1 的角度模式。
4.3 各种动作函数（gait primitives）
straightline()
调用 homePose()，把蛇复位回直线姿态。
Cshape()
将所有舵机设置为 60°，整体向一侧弯曲成 C 形（简单示范姿势）。
ubend()
计算中间关节（中间 1 或 2 个），设为 0°，其它为 90°；形成 U 形。
ring()
利用正多边形内角公式计算弯曲角度，让每一节弯得差不多，整体接近一个环。
slither(int offset, int Amplitude, int Speed, float Wavelengths)
关键蛇形 gait 函数：
时间上：随时间 t 变化的正弦项 sin(Speed * t + phase)；
空间上：每个关节相位差为 j * Wavelengths * Shift；
offset 控制整体偏置（用于左/右转弯）；
Amplitude 控制摆动幅度；
Speed 控制波传播速度；
Wavelengths 控制整条蛇上铺了多少个波长。
staticWave(int offset, int Amplitude, float Wavelengths)
只在空间上有波形，不随时间变化；常用于展示不同弯曲造型。
InchWorm()
利用 applyHump() 在身体上从尾到头依次“滚动”一个小拱，模仿尺蠖的前进方式。
4.4 串口菜单与控制逻辑
printHelp()
在串口打印所有可用命令和说明。
handleKey(char c)
根据收到的单个字符 c 选择对应 gait 函数；
包含对 r、0、1～9、h 的处理。
setup()
初始化串口：Serial.begin(115200)；
计算 Shift = 2 * pi / TotalNumberofServos；
附加各个 Servo 到对应引脚；
调用 homePose() 做一次上电复位；
打印帮助菜单。
loop()
检查 Serial.available()；
有数据就读取一个字符：
如果是 \n 或 \r，忽略；
否则丢给 handleKey(c) 处理。
各个动作函数内部都包含自己的 delay()，因此 loop 本身是事件驱动而非周期调度。

5. 校准与调试建议
初次装配后：
烧录主程序；
通电 → 串口输入 r（Raw Reset）→ 所有关节转到 90°；
观察蛇是否大致成直线，如不直，可以卸下个别舵臂重新插。
微调零点偏差：
如果某一节还是偏一点点，可以修改：
int servoOffset[6] = {0, 0, 0, 0, 0, 0};
例如改成 {0, 2, -3, 0, 1, 0}，再重新上传程序；
然后按 0 或 9 看 Home 姿态是否直了。
动作测试顺序：
先测试静态动作：6（U 形）、7（ring）、8（C 形）、9（直线）；
再试动态动作：1～5，观察是否有卡爪、线缠绕等问题。
如出现舵机抖动严重、温度异常升高，请减小 Amplitude 或检查舵机电源是否足够。

6. 后续扩展（面向 LLM / 远程控制）
虽然当前版本只通过串口手动输入字符控制动作，但程序结构已经为后续扩展预留空间：
可以在 loop() 中改为解析完整字符串命令（例如 SLITHER 0 35 2 1.5~）；
由 PC / Node-RED / LLM 生成这些命令，通过 USB 串口发送给 Arduino；
Arduino 只需把 handleKey 的逻辑扩展为 handleCommandString(String cmd) 即可。
这样，当前 README 和主程序既可以作为单独的实验，也可以作为后续“LLM + Snake”大项目的底层控制模块。
