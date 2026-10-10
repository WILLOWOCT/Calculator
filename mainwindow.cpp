#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QSizePolicy>
#include <QKeyEvent>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFixedSize(400, 580);

    // ===== 布局修正：显示区高度固定，按钮区垂直撑满窗口 =====
    // 原问题：QLabel 默认可无限拉伸、QPushButton 垂直方向固定不伸展，
    // 导致窗口多余的高度全部被显示区占据，按钮挤在下方、上方大片空白。
    ui->lblHistory->setFixedHeight(26);          // 历史行固定 26px
    ui->lblDisplay->setFixedHeight(110);         // 主显示行固定 110px
    const auto allButtons = findChildren<QPushButton *>();   // 所有按钮垂直方向改为可伸展
    for (QPushButton *btn : allButtons) {
        QSizePolicy sp = btn->sizePolicy();
        sp.setVerticalPolicy(QSizePolicy::Expanding);
        btn->setSizePolicy(sp);
    }

    // ===== 使用样式表(QSS)改善界面：仿手机计算器的浅色圆角风格 =====
    // 1. 显示区：大字号深色文字；历史行小号灰色
    // 2. 数字键：白底圆角；运算符键：淡蓝底深蓝字；等号键：深蓝底白字；C 键：淡红底红字
    setStyleSheet(QStringLiteral(R"(
        MainWindow { background: #eef1f6; }
        #lblDisplay {
            font-size: 44px; font-weight: 600; color: #1c1e21;
            padding-right: 6px;
        }
        #lblHistory {
            font-size: 15px; color: #90949c; padding-right: 8px;
        }
        QPushButton {
            font-size: 22px; font-weight: 500; color: #1c1e21;
            border: none; border-radius: 10px; background: #ffffff;
        }
        QPushButton:hover   { background: #e4e8ef; }
        QPushButton:pressed { background: #d3d9e3; }
        QPushButton#btnAdd, QPushButton#btnSub,
        QPushButton#btnMul, QPushButton#btnDiv {
            background: #dfe9ff; color: #2b5fd9; font-size: 26px;
        }
        QPushButton#btnEqual { background: #2b5fd9; color: #ffffff; font-size: 26px; }
        QPushButton#btnEqual:hover  { background: #2453c4; }
        QPushButton#btnClear { background: #ffe8e8; color: #d93025; }
    )"));

    // 所有按钮统一连接到 onKeyClicked，为鼠标与键盘共用同一处理逻辑做准备
    const auto buttons = findChildren<QPushButton *>();
    for (QPushButton *btn : buttons)
        connect(btn, &QPushButton::clicked, this, &MainWindow::onKeyClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------- 键盘事件：映射为与按钮一致的 key 字符串，复用 processKey ----------
// 鼠标输入与键盘输入完全共用同一套处理逻辑，保证行为一致
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QString key;
    switch (event->key()) {
    case Qt::Key_0:            key = "0";  break;
    case Qt::Key_1:            key = "1";  break;
    case Qt::Key_2:            key = "2";  break;
    case Qt::Key_3:            key = "3";  break;
    case Qt::Key_4:            key = "4";  break;
    case Qt::Key_5:            key = "5";  break;
    case Qt::Key_6:            key = "6";  break;
    case Qt::Key_7:            key = "7";  break;
    case Qt::Key_8:            key = "8";  break;
    case Qt::Key_9:            key = "9";  break;
    case Qt::Key_Period:
    case Qt::Key_Comma:        key = ".";  break;
    case Qt::Key_Plus:         key = "+";  break;
    case Qt::Key_Minus:        key = "-";  break;
    case Qt::Key_Asterisk:     key = "×";  break;
    case Qt::Key_Slash:        key = "÷";  break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Equal:        key = "=";  break;
    case Qt::Key_Backspace:    key = "⌫";  break;
    case Qt::Key_Escape:
    case Qt::Key_Delete:       key = "C";  break;
    case Qt::Key_F9:           key = "±";  break;
    default:                   QMainWindow::keyPressEvent(event); return;
    }
    processKey(key);
}

void MainWindow::onKeyClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    processKey(btn->text());
}

// ---------- 按键分发：鼠标与键盘共用的统一入口 ----------
void MainWindow::processKey(const QString &key)
{
    if (key >= "0" && key <= "9")
        inputDigit(key);
    else if (key == ".")
        inputDot();
    else if (key == "+" || key == "-" || key == "×" || key == "÷")
        inputOperator(key);
    else if (key == "=")
        calculateEqual();
    else if (key == "C")
        clearAll();
    else if (key == "⌫")
        backspace();
    else if (key == "±")
        toggleSign();
}

// ---------- 数字输入 ----------
void MainWindow::inputDigit(const QString &digit)
{
    if (m_error)
        clearAll();                    // 错误状态下按数字：直接开始新的计算
    if (m_waitingForOperand) {         // 开始输入一个新操作数
        m_display = digit;
        m_waitingForOperand = false;
    } else {
        // 前导零处理：显示为 0 时再按数字直接替换
        if (m_display == "0")
            m_display = digit;
        else if (m_display.length() < 15)      // 限制输入长度，防止溢出显示
            m_display += digit;
    }
    refreshDisplay();
}

// ---------- 小数点输入 ----------
void MainWindow::inputDot()
{
    if (m_error)
        clearAll();
    if (m_waitingForOperand) {         // 新操作数直接按小数点：补全为 "0."
        m_display = QStringLiteral("0.");
        m_waitingForOperand = false;
    } else if (!m_display.contains('.')) {
        // 只有当前操作数还没有小数点时才允许输入，避免 "1.2.3" 这类异常
        m_display += '.';
    }
    refreshDisplay();
}

// ---------- 操作符输入（+ - × ÷） ----------
void MainWindow::inputOperator(const QString &op)
{
    if (m_error)
        return;                        // 错误状态下操作符无效，需先按 C 恢复
    if (!m_pendingOperator.isEmpty() && !m_waitingForOperand) {
        // 已有两个操作数还按操作符 → 连续运算：先算出前一步结果
        if (!computeStep())
            return;
    } else if (m_pendingOperator.isEmpty()) {
        m_storedOperand = m_display;   // 记录第一个操作数
    } else {
        // pendingOperator 非空且尚未输入第二个操作数 → 连续按操作符，替换之
    }
    m_pendingOperator = op;
    m_waitingForOperand = true;        // 等待第二个操作数
    refreshDisplay();
}

// ---------- 等号 ----------
void MainWindow::calculateEqual()
{
    if (m_error)
        return;
    if (m_pendingOperator.isEmpty())   // 没有待计算的运算，忽略
        return;
    if (m_waitingForOperand)           // 第二个操作数还没输入，忽略
        return;
    computeStep();
    m_pendingOperator.clear();
    m_waitingForOperand = true;        // 计算完成后：继续按数字 → 新操作数
                                       //           继续按操作符 → 用结果连算
    refreshDisplay();
}

// ---------- 执行一步四则运算 ----------
bool MainWindow::computeStep()
{
    double a = m_storedOperand.toDouble();
    double b = m_display.toDouble();
    double r = 0;

    if (m_pendingOperator == "+")  r = a + b;
    else if (m_pendingOperator == "-") r = a - b;
    else if (m_pendingOperator == "×") r = a * b;
    else if (m_pendingOperator == "÷") {
        if (qFuzzyIsNull(b)) {         // 除零检查：b 为 0 时进入错误状态
            showError(QStringLiteral("错误：除数不能为零"));
            return false;
        }
        r = a / b;
    }

    m_display = formatResult(r);
    m_storedOperand.clear();
    return true;
}

// ---------- 退格 ----------
void MainWindow::backspace()
{
    if (m_error || m_waitingForOperand)
        return;                        // 错误状态/刚出结果时退格无意义
    m_display.chop(1);
    if (m_display.isEmpty() || m_display == "-")
        m_display = QStringLiteral("0");
    refreshDisplay();
}

// ---------- 清除全部 ----------
void MainWindow::clearAll()
{
    m_display = QStringLiteral("0");
    m_storedOperand.clear();
    m_pendingOperator.clear();
    m_waitingForOperand = true;
    m_error = false;
    refreshDisplay();
}

// ---------- 正负号 ----------
void MainWindow::toggleSign()
{
    if (m_error || m_waitingForOperand)
        return;
    if (m_display.startsWith('-'))
        m_display.remove(0, 1);
    else if (m_display != "0")
        m_display.prepend('-');
    refreshDisplay();
}

// ---------- 错误状态 ----------
void MainWindow::showError(const QString &msg)
{
    m_display = msg;
    m_error = true;
    refreshDisplay();
}

// ---------- 结果格式化 ----------
QString MainWindow::formatResult(double v) const
{
    if (std::isnan(v) || std::isinf(v))
        return QStringLiteral("错误：结果溢出");
    QString s = QString::number(v, 'g', 13);
    if (s == "-0")
        s = QStringLiteral("0");
    return s;
}

// ---------- 刷新显示 ----------
void MainWindow::refreshDisplay()
{
    ui->lblDisplay->setText(m_display.isEmpty() ? QStringLiteral("0") : m_display);
    // 历史行提示当前状态：如 "12 + "
    if (!m_pendingOperator.isEmpty() && !m_error)
        ui->lblHistory->setText(m_storedOperand + ' ' + m_pendingOperator);
    else
        ui->lblHistory->setText(QString());
}
