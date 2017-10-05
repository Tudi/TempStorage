/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDateEdit>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QMainWindow>
#include <QtGui/QPushButton>
#include <QtGui/QTableWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QLabel *L_ClientSeed;
    QPushButton *b_BrowseClientSeed;
    QDateEdit *dt_EndDate;
    QLabel *L_FeatureIdPicker;
    QLabel *L_EndDate;
    QDateEdit *dt_StartDate;
    QTextEdit *t_LicenseSeedPath;
    QPushButton *b_RemoveSelectedFromLicense;
    QLabel *L_ProductIdPicker;
    QLabel *L_StartDate;
    QPushButton *b_AddToLicense;
    QPushButton *b_GenerateLicense;
    QComboBox *CB_ProductIdPicker;
    QComboBox *CB_FeatureIdPicker;
    QTableWidget *table_LicenseContent;
    QLabel *label;
    QTextEdit *t_GraceDuration;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(545, 379);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        L_ClientSeed = new QLabel(centralWidget);
        L_ClientSeed->setObjectName(QString::fromUtf8("L_ClientSeed"));
        L_ClientSeed->setGeometry(QRect(20, 20, 61, 16));
        b_BrowseClientSeed = new QPushButton(centralWidget);
        b_BrowseClientSeed->setObjectName(QString::fromUtf8("b_BrowseClientSeed"));
        b_BrowseClientSeed->setGeometry(QRect(430, 10, 101, 41));
        dt_EndDate = new QDateEdit(centralWidget);
        dt_EndDate->setObjectName(QString::fromUtf8("dt_EndDate"));
        dt_EndDate->setGeometry(QRect(310, 70, 110, 22));
        L_FeatureIdPicker = new QLabel(centralWidget);
        L_FeatureIdPicker->setObjectName(QString::fromUtf8("L_FeatureIdPicker"));
        L_FeatureIdPicker->setGeometry(QRect(250, 140, 51, 21));
        L_EndDate = new QLabel(centralWidget);
        L_EndDate->setObjectName(QString::fromUtf8("L_EndDate"));
        L_EndDate->setGeometry(QRect(250, 70, 46, 13));
        dt_StartDate = new QDateEdit(centralWidget);
        dt_StartDate->setObjectName(QString::fromUtf8("dt_StartDate"));
        dt_StartDate->setGeometry(QRect(110, 70, 110, 22));
        t_LicenseSeedPath = new QTextEdit(centralWidget);
        t_LicenseSeedPath->setObjectName(QString::fromUtf8("t_LicenseSeedPath"));
        t_LicenseSeedPath->setGeometry(QRect(110, 10, 311, 41));
        b_RemoveSelectedFromLicense = new QPushButton(centralWidget);
        b_RemoveSelectedFromLicense->setObjectName(QString::fromUtf8("b_RemoveSelectedFromLicense"));
        b_RemoveSelectedFromLicense->setGeometry(QRect(430, 180, 101, 23));
        L_ProductIdPicker = new QLabel(centralWidget);
        L_ProductIdPicker->setObjectName(QString::fromUtf8("L_ProductIdPicker"));
        L_ProductIdPicker->setGeometry(QRect(20, 140, 51, 21));
        L_StartDate = new QLabel(centralWidget);
        L_StartDate->setObjectName(QString::fromUtf8("L_StartDate"));
        L_StartDate->setGeometry(QRect(20, 70, 71, 16));
        b_AddToLicense = new QPushButton(centralWidget);
        b_AddToLicense->setObjectName(QString::fromUtf8("b_AddToLicense"));
        b_AddToLicense->setGeometry(QRect(430, 140, 101, 23));
        b_GenerateLicense = new QPushButton(centralWidget);
        b_GenerateLicense->setObjectName(QString::fromUtf8("b_GenerateLicense"));
        b_GenerateLicense->setGeometry(QRect(200, 330, 91, 23));
        CB_ProductIdPicker = new QComboBox(centralWidget);
        CB_ProductIdPicker->setObjectName(QString::fromUtf8("CB_ProductIdPicker"));
        CB_ProductIdPicker->setGeometry(QRect(110, 140, 111, 22));
        CB_FeatureIdPicker = new QComboBox(centralWidget);
        CB_FeatureIdPicker->setObjectName(QString::fromUtf8("CB_FeatureIdPicker"));
        CB_FeatureIdPicker->setGeometry(QRect(310, 140, 111, 22));
        table_LicenseContent = new QTableWidget(centralWidget);
        table_LicenseContent->setObjectName(QString::fromUtf8("table_LicenseContent"));
        table_LicenseContent->setGeometry(QRect(20, 180, 401, 131));
        table_LicenseContent->setLineWidth(2);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(20, 110, 91, 16));
        t_GraceDuration = new QTextEdit(centralWidget);
        t_GraceDuration->setObjectName(QString::fromUtf8("t_GraceDuration"));
        t_GraceDuration->setGeometry(QRect(110, 100, 104, 31));
        t_GraceDuration->setInputMethodHints(Qt::ImhPreferNumbers);
        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        L_ClientSeed->setText(QApplication::translate("MainWindow", "Client seed", 0, QApplication::UnicodeUTF8));
        b_BrowseClientSeed->setText(QApplication::translate("MainWindow", "Browse", 0, QApplication::UnicodeUTF8));
        L_FeatureIdPicker->setText(QApplication::translate("MainWindow", "Feature", 0, QApplication::UnicodeUTF8));
        L_EndDate->setText(QApplication::translate("MainWindow", "End date", 0, QApplication::UnicodeUTF8));
        b_RemoveSelectedFromLicense->setText(QApplication::translate("MainWindow", "Remove selected", 0, QApplication::UnicodeUTF8));
        L_ProductIdPicker->setText(QApplication::translate("MainWindow", "Product", 0, QApplication::UnicodeUTF8));
        L_StartDate->setText(QApplication::translate("MainWindow", "Start Date", 0, QApplication::UnicodeUTF8));
        b_AddToLicense->setText(QApplication::translate("MainWindow", "Add to license", 0, QApplication::UnicodeUTF8));
        b_GenerateLicense->setText(QApplication::translate("MainWindow", "Generate license", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Grace time(days)", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
