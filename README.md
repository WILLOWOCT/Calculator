# Calculator — 带键盘事件的计算器（Qt 实验一）

东莞理工学院 计算机科学与技术学院《Qt 应用程序开发》实验一。

一个支持鼠标点击与键盘输入的四则运算计算器，基于 Qt Widgets + UI 设计器开发。

## 姓名 / 学号

- 姓名：赖柳燕
- 学号：2024414290114

## 功能

- 四则运算（+ - × ÷）、小数输入、退格（⌫）、清除（C）、正负号（±）
- 键盘输入：数字键直接输入；`.` 或 `,` 小数点；`+ - * /` 四则；
  `Enter` / `=` 等号；`Backspace` 退格；`Esc` / `Delete` 清除；`F9` 正负号
- 鼠标与键盘输入复用同一套处理逻辑（`processKey()`），行为完全一致
- 异常输入处理：
  - 同一操作数重复输入小数点（`1.2.` 不再追加）
  - 连续操作符自动替换（`5 + × 3` 视为 `5 × 3`）
  - 除数为 0 时显示"错误：除数不能为零"，按 `C` 或重新输入数字恢复
  - 连续运算（`1+2+3=` 按顺序计算得 6）
  - 计算完成后按数字开始新运算、按操作符用结果连算
  - 前导零处理（显示为 0 时按数字直接替换）

## 构建

使用 Qt 6（Qt Creator）打开 `Calculator.pro`，选择 Desktop Qt 套件，
Ctrl+R 编译运行；或命令行：

```
qmake Calculator.pro
mingw32-make
```

## 代码结构

- `mainwindow.ui` — UI 设计器文件：显示区（lblHistory / lblDisplay）+ QGridLayout 16 个按键
- `mainwindow.cpp` — 核心逻辑：processKey() 统一分发，inputDigit / inputDot /
  inputOperator / calculateEqual / backspace / clearAll / toggleSign
- `keyPressEvent()` — 键盘事件入口，映射后与鼠标共用 processKey()
