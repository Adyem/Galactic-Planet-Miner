#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow : public QMainWindow {
    Q_OBJECT  // This requires MOC processing
public:
    explicit MainWindow(QWidget *parent = nullptr);
};

#endif

