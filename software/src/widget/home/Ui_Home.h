/********************************************************************************
** Form generated from reading UI file 'home.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HOME_H
#define UI_HOME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Home
{
public:
    QWidget *centralWidget;
    QVBoxLayout *vlCentralWidget;
    QSpacerItem *topSpacer;
    QPushButton *quickAutoBtn;
    QPushButton *fullAutoBtn;
    QPushButton *manualBtn;
    QPushButton *modifyBtn;
    QPushButton *systemdBtn;
    QSpacerItem *botSpacer;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *Home)
    {
        if (Home->objectName().isEmpty())
            Home->setObjectName(QString::fromUtf8("Home"));
        Home->resize(800, 600);
        centralWidget = new QWidget(Home);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        vlCentralWidget = new QVBoxLayout(centralWidget);
        vlCentralWidget->setObjectName(QString::fromUtf8("vlCentralWidget"));
        topSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vlCentralWidget->addItem(topSpacer);

        quickAutoBtn = new QPushButton(centralWidget);
        quickAutoBtn->setObjectName(QString::fromUtf8("quickAutoBtn"));

        vlCentralWidget->addWidget(quickAutoBtn);

        fullAutoBtn = new QPushButton(centralWidget);
        fullAutoBtn->setObjectName(QString::fromUtf8("fullAutoBtn"));

        vlCentralWidget->addWidget(fullAutoBtn);

        manualBtn = new QPushButton(centralWidget);
        manualBtn->setObjectName(QString::fromUtf8("manualBtn"));

        vlCentralWidget->addWidget(manualBtn);

        modifyBtn = new QPushButton(centralWidget);
        modifyBtn->setObjectName(QString::fromUtf8("modifyBtn"));

        vlCentralWidget->addWidget(modifyBtn);

        systemdBtn = new QPushButton(centralWidget);
        systemdBtn->setObjectName(QString::fromUtf8("systemdBtn"));

        vlCentralWidget->addWidget(systemdBtn);

        botSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vlCentralWidget->addItem(botSpacer);

        Home->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(Home);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        Home->setStatusBar(statusBar);

        retranslateUi(Home);
        QObject::connect(quickAutoBtn, SIGNAL(released()), Home, SLOT(quickAutoPressed()));
        QObject::connect(fullAutoBtn, SIGNAL(released()), Home, SLOT(fullAutoPressed()));
        QObject::connect(modifyBtn, SIGNAL(released()), Home, SLOT(modifyPressed()));
        QObject::connect(manualBtn, SIGNAL(released()), Home, SLOT(manualPressed()));
        QObject::connect(systemdBtn, SIGNAL(released()), Home, SLOT(systemdPressed()));

        QMetaObject::connectSlotsByName(Home);
    } // setupUi

    void retranslateUi(QMainWindow *Home)
    {
        Home->setWindowTitle(QCoreApplication::translate("Home", "MainWindow", nullptr));
        quickAutoBtn->setText(QCoreApplication::translate("Home", "Quick automatic configuration", nullptr));
        fullAutoBtn->setText(QCoreApplication::translate("Home", "Full automatic configuration", nullptr));
        manualBtn->setText(QCoreApplication::translate("Home", "Manual configuration", nullptr));
        modifyBtn->setText(QCoreApplication::translate("Home", "Run acutal configuration", nullptr));
        systemdBtn->setText(QCoreApplication::translate("Home", "Start infrared emitter at boot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Home: public Ui_Home {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HOME_H
