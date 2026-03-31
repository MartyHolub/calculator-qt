#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onDigitClicked();
    void onOperatorClicked();
    void onEqualsClicked();
    void onClearClicked();
    void onDecimalClicked();
    void onToggleSignClicked();

private:
    QLineEdit *display;

    double      m_operand    = 0.0;
    QString     m_pendingOp;
    bool        m_waitingForOperand = false;

    void        setDisplay(double value);
};

#endif // MAINWINDOW_H
