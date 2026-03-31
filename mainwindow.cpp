#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Calculator");
    resize(400, 300);

    button = new QPushButton("Click me", this);
    button->setGeometry(150, 120, 100, 40);

    connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
}

MainWindow::~MainWindow() {}

void MainWindow::onButtonClicked()
{
    button->setText("Clicked!");
}
