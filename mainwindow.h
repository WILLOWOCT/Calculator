#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QKeyEvent;

/**
 * @brief 带键盘事件的计算器（实验一）
 *
 * 设计要点：
 *  1. 鼠标点击与键盘输入统一走 processKey()，共用同一套处理逻辑；
 *  2. 用 m_waitingForOperand 管理输入状态，避免同一操作数重复输入小数点、
 *     前导零等异常；
 *  3. m_storedOperand / m_pendingOperator / m_display 构成双操作数状态机，
 *     支持连续运算（如 1+2+3=6）；
 *  4. 除零等异常以错误状态 m_error 管理，按 C 或重新输入数字可恢复。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;   // 键盘事件入口

private slots:
    void onKeyClicked();        // 所有鼠标按钮的统一槽（复用 processKey）

private:
    // ---------- 按键分发（鼠标/键盘共用） ----------
    void processKey(const QString &key);
    void inputDigit(const QString &digit);   // 数字 0-9
    void inputDot();                         // 小数点
    void inputOperator(const QString &op);   // + - × ÷
    void calculateEqual();                   // =
    void backspace();                        // 退格
    void clearAll();                         // C 清除全部
    void toggleSign();                       // ± 正负号

    // ---------- 辅助 ----------
    bool computeStep();                      // 执行一步四则运算（含除零检查）
    void showError(const QString &msg);      // 进入错误状态
    void refreshDisplay();                   // 刷新显示区与历史行
    QString formatResult(double v) const;    // 结果格式化

    Ui::MainWindow *ui;

    // ---------- 计算状态 ----------
    QString m_display;          // 显示区当前内容（当前操作数或结果）
    QString m_storedOperand;    // 第一个操作数
    QString m_pendingOperator;  // 待执行的操作符：+ - × ÷
    bool    m_waitingForOperand = true;  // true：下一个数字开始新操作数
    bool    m_error = false;             // 错误状态（如除数为零）
};

#endif // MAINWINDOW_H
