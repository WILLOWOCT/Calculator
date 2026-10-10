#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 所有按钮统一连接到 onKeyClicked，为鼠标与键盘共用同一处理逻辑做准备
    const auto buttons = findChildren<QPushButton *>();
    for (QPushButton *btn : buttons)
        connect(btn, &QPushButton::clicked, this, &MainWindow::onKeyClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onKeyClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (!btn)
        return;
    processKey(btn->text());
}

void MainWindow::processKey(const QString &key)
{
    if (key >= "0" && key <= "9") {                 // 数字
        m_display = (m_display == "0") ? key : m_display + key;
    } else if (key == ".") {                        // 小数点（暂未检查重复）
        m_display += key;
    } else if (key == "+" || key == "-" || key == "×" || key == "÷") {
        m_storedOperand = m_display;                // 保存第一个操作数
        m_pendingOperator = key;
        m_display.clear();                          // 等待第二个操作数
    } else if (key == "=") {
        if (m_pendingOperator.isEmpty() || m_display.isEmpty())
            return;
        double a = m_storedOperand.toDouble();
        double b = m_display.toDouble();
        double r = 0;
        if (m_pendingOperator == "+")  r = a + b;
        else if (m_pendingOperator == "-") r = a - b;
        else if (m_pendingOperator == "×") r = a * b;
        else if (m_pendingOperator == "÷") r = a / b;   // 暂未检查除零（已知问题）
        m_display = QString::number(r, 'g', 13);
        m_storedOperand.clear();
        m_pendingOperator.clear();
    }
    refreshDisplay();
}

void MainWindow::refreshDisplay()
{
    ui->lblDisplay->setText(m_display.isEmpty() ? QStringLiteral("0") : m_display);
}
