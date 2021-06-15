#ifndef CON_HOME_HPP
#define CON_HOME_HPP

#include <QtWidgets/QMainWindow>

#include "Ui_Home.h"

class Con_Home : public QMainWindow {
  Q_OBJECT
  Ui_Home *ui;

public:
  explicit Con_Home(QWidget *parent = nullptr);
  virtual ~Con_Home();

private slots:
  void quickAutoPressed() {
  }

  void fullAutoPressed() {}

  void manualPressed() {}

  void modifyPressed() {}

  void systemdPressed() {}
};

#endif