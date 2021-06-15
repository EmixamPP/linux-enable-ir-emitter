#include <QtWidgets/QApplication>

#include "widget/home/Con_Home.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  Con_Home *home = new Con_Home;
  home->show();
  app.exec();
  delete home;
  return 0;
}