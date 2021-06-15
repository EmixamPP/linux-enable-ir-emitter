#include "Con_Home.hpp"

Con_Home::Con_Home(QWidget *parent) : QMainWindow(parent), ui(new Ui_Home) { ui->setupUi(this); }
Con_Home::~Con_Home() { delete ui; }