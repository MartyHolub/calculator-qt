#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QFont>
#include <cmath>

// Helper: create a styled calculator button
static QPushButton *makeButton(const QString &text,
                               const QString &bg = "#505050",
                               const QString &fg = "#ffffff")
{
    auto *btn = new QPushButton(text);
    btn->setMinimumSize(70, 60);
    btn->setFont(QFont("Arial", 16));
    btn->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border-radius: 6px; }"
        "QPushButton:pressed { background-color: #888888; }").arg(bg, fg));
    return btn;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Calculator");

    // Central widget + main layout
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(6);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Display
    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setFont(QFont("Arial", 28, QFont::Bold));
    display->setMinimumHeight(70);
    display->setStyleSheet(
        "QLineEdit { background-color: #1c1c1c; color: #ffffff; "
        "border: none; border-radius: 6px; padding: 0 10px; }");
    mainLayout->addWidget(display);

    // Button grid
    auto *grid = new QGridLayout();
    grid->setSpacing(6);

    // Row 0: C, +/-, %, ÷
    auto *btnC   = makeButton("C",   "#a5a5a5", "#000000");
    auto *btnPM  = makeButton("+/-", "#a5a5a5", "#000000");
    auto *btnPct = makeButton("%",   "#a5a5a5", "#000000");
    auto *btnDiv = makeButton("÷",   "#ff9f0a");

    // Row 1: 7 8 9 ×
    auto *btn7   = makeButton("7");
    auto *btn8   = makeButton("8");
    auto *btn9   = makeButton("9");
    auto *btnMul = makeButton("×",   "#ff9f0a");

    // Row 2: 4 5 6 −
    auto *btn4   = makeButton("4");
    auto *btn5   = makeButton("5");
    auto *btn6   = makeButton("6");
    auto *btnSub = makeButton("−",   "#ff9f0a");

    // Row 3: 1 2 3 +
    auto *btn1   = makeButton("1");
    auto *btn2   = makeButton("2");
    auto *btn3   = makeButton("3");
    auto *btnAdd = makeButton("+",   "#ff9f0a");

    // Row 4: 0 (wide) . =
    auto *btn0   = makeButton("0");
    auto *btnDot = makeButton(".");
    auto *btnEq  = makeButton("=",   "#ff9f0a");

    // btn0 spans 2 columns
    btn0->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    grid->addWidget(btnC,   0, 0);
    grid->addWidget(btnPM,  0, 1);
    grid->addWidget(btnPct, 0, 2);
    grid->addWidget(btnDiv, 0, 3);

    grid->addWidget(btn7,   1, 0);
    grid->addWidget(btn8,   1, 1);
    grid->addWidget(btn9,   1, 2);
    grid->addWidget(btnMul, 1, 3);

    grid->addWidget(btn4,   2, 0);
    grid->addWidget(btn5,   2, 1);
    grid->addWidget(btn6,   2, 2);
    grid->addWidget(btnSub, 2, 3);

    grid->addWidget(btn1,   3, 0);
    grid->addWidget(btn2,   3, 1);
    grid->addWidget(btn3,   3, 2);
    grid->addWidget(btnAdd, 3, 3);

    grid->addWidget(btn0,   4, 0, 1, 2);
    grid->addWidget(btnDot, 4, 2);
    grid->addWidget(btnEq,  4, 3);

    mainLayout->addLayout(grid);

    // Digit buttons
    for (auto *b : {btn0, btn1, btn2, btn3, btn4, btn5, btn6, btn7, btn8, btn9})
        connect(b, &QPushButton::clicked, this, &MainWindow::onDigitClicked);

    // Operator buttons — store the operator symbol in objectName
    btnAdd->setObjectName("+");
    btnSub->setObjectName("-");
    btnMul->setObjectName("*");
    btnDiv->setObjectName("/");
    btnPct->setObjectName("%");
    for (auto *b : {btnAdd, btnSub, btnMul, btnDiv, btnPct})
        connect(b, &QPushButton::clicked, this, &MainWindow::onOperatorClicked);

    connect(btnEq,  &QPushButton::clicked, this, &MainWindow::onEqualsClicked);
    connect(btnC,   &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(btnDot, &QPushButton::clicked, this, &MainWindow::onDecimalClicked);
    connect(btnPM,  &QPushButton::clicked, this, &MainWindow::onToggleSignClicked);

    adjustSize();
    setFixedSize(sizeHint());
}

MainWindow::~MainWindow() {}

// ── helpers ──────────────────────────────────────────────────────────────────

void MainWindow::setDisplay(double value)
{
    // Show integer if the value has no fractional part
    if (value == std::floor(value) && std::abs(value) < 1e15)
        display->setText(QString::number(static_cast<long long>(value)));
    else
        display->setText(QString::number(value, 'g', 12));
}

// ── slots ─────────────────────────────────────────────────────────────────────

void MainWindow::onDigitClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    const QString digit = btn->text();

    if (m_waitingForOperand) {
        display->setText(digit);
        m_waitingForOperand = false;
    } else {
        const QString current = display->text();
        display->setText(current == "0" ? digit : current + digit);
    }
}

void MainWindow::onDecimalClicked()
{
    if (m_waitingForOperand) {
        display->setText("0.");
        m_waitingForOperand = false;
        return;
    }
    if (!display->text().contains('.'))
        display->setText(display->text() + ".");
}

void MainWindow::onOperatorClicked()
{
    auto *btn = qobject_cast<QPushButton *>(sender());
    const QString op = btn->objectName();
    const double current = display->text().toDouble();

    if (op == "%") {
        // Percentage: just divide by 100
        setDisplay(current / 100.0);
        m_waitingForOperand = true;
        return;
    }

    // If we already have a pending operation, evaluate it first
    if (!m_pendingOp.isEmpty() && !m_waitingForOperand)
        onEqualsClicked();

    m_operand  = display->text().toDouble();
    m_pendingOp = op;
    m_waitingForOperand = true;
}

void MainWindow::onEqualsClicked()
{
    if (m_pendingOp.isEmpty())
        return;

    const double rhs = display->text().toDouble();
    double result = 0.0;

    if (m_pendingOp == "+")       result = m_operand + rhs;
    else if (m_pendingOp == "-")  result = m_operand - rhs;
    else if (m_pendingOp == "*")  result = m_operand * rhs;
    else if (m_pendingOp == "/") {
        if (rhs == 0.0) {
            display->setText("Error");
            m_pendingOp.clear();
            return;
        }
        result = m_operand / rhs;
    }

    setDisplay(result);
    m_pendingOp.clear();
    m_waitingForOperand = true;
}

void MainWindow::onClearClicked()
{
    display->setText("0");
    m_operand = 0.0;
    m_pendingOp.clear();
    m_waitingForOperand = false;
}

void MainWindow::onToggleSignClicked()
{
    const double value = display->text().toDouble();
    setDisplay(-value);
}

