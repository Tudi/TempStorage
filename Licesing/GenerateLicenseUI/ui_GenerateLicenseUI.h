/********************************************************************************
** Form generated from reading UI file 'GenerateLicenseUI.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_GENERATELICENSEUI_H
#define UI_GENERATELICENSEUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDateEdit>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_LicenseGenerate
{
public:
    QLabel *L_ClientSeed;
    QTextEdit *t_LicenseSeedPath;
    QListView *lv_LicenseContent;
    QLabel *L_StartDate;
    QDateEdit *dt_EndDate;
    QPushButton *b_AddToLicense;
    QLabel *L_FeatureIdPicker;
    QLabel *L_OutputFile;
    QComboBox *CB_ProductIdPicker;
    QDateEdit *dt_StartDate;
    QPushButton *b_BrowseClientSeed;
    QPushButton *b_RemoveSelectedFromLicense;
    QComboBox *CB_FeatureIdPicker;
    QTextEdit *t_OutputLicensePath;
    QLabel *L_EndDate;
    QLabel *L_ProductIdPicker;
    QPushButton *b_GenerateLicense;

    void setupUi(QFrame *LicenseGenerate)
    {
        if (LicenseGenerate->objectName().isEmpty())
            LicenseGenerate->setObjectName(QString::fromUtf8("LicenseGenerate"));
        LicenseGenerate->resize(536, 411);
        LicenseGenerate->setFrameShape(QFrame::StyledPanel);
        LicenseGenerate->setFrameShadow(QFrame::Raised);
        L_ClientSeed = new QLabel(LicenseGenerate);
        L_ClientSeed->setObjectName(QString::fromUtf8("L_ClientSeed"));
        L_ClientSeed->setGeometry(QRect(16, 20, 61, 16));
        t_LicenseSeedPath = new QTextEdit(LicenseGenerate);
        t_LicenseSeedPath->setObjectName(QString::fromUtf8("t_LicenseSeedPath"));
        t_LicenseSeedPath->setGeometry(QRect(106, 10, 311, 31));
        lv_LicenseContent = new QListView(LicenseGenerate);
        lv_LicenseContent->setObjectName(QString::fromUtf8("lv_LicenseContent"));
        lv_LicenseContent->setGeometry(QRect(16, 150, 401, 192));
        L_StartDate = new QLabel(LicenseGenerate);
        L_StartDate->setObjectName(QString::fromUtf8("L_StartDate"));
        L_StartDate->setGeometry(QRect(16, 70, 71, 16));
        dt_EndDate = new QDateEdit(LicenseGenerate);
        dt_EndDate->setObjectName(QString::fromUtf8("dt_EndDate"));
        dt_EndDate->setGeometry(QRect(306, 70, 110, 22));
        b_AddToLicense = new QPushButton(LicenseGenerate);
        b_AddToLicense->setObjectName(QString::fromUtf8("b_AddToLicense"));
        b_AddToLicense->setGeometry(QRect(426, 110, 101, 23));
        L_FeatureIdPicker = new QLabel(LicenseGenerate);
        L_FeatureIdPicker->setObjectName(QString::fromUtf8("L_FeatureIdPicker"));
        L_FeatureIdPicker->setGeometry(QRect(246, 110, 51, 21));
        L_OutputFile = new QLabel(LicenseGenerate);
        L_OutputFile->setObjectName(QString::fromUtf8("L_OutputFile"));
        L_OutputFile->setGeometry(QRect(16, 380, 61, 16));
        CB_ProductIdPicker = new QComboBox(LicenseGenerate);
        CB_ProductIdPicker->setObjectName(QString::fromUtf8("CB_ProductIdPicker"));
        CB_ProductIdPicker->setGeometry(QRect(106, 110, 111, 22));
        dt_StartDate = new QDateEdit(LicenseGenerate);
        dt_StartDate->setObjectName(QString::fromUtf8("dt_StartDate"));
        dt_StartDate->setGeometry(QRect(106, 70, 110, 22));
        b_BrowseClientSeed = new QPushButton(LicenseGenerate);
        b_BrowseClientSeed->setObjectName(QString::fromUtf8("b_BrowseClientSeed"));
        b_BrowseClientSeed->setGeometry(QRect(426, 10, 101, 31));
        b_RemoveSelectedFromLicense = new QPushButton(LicenseGenerate);
        b_RemoveSelectedFromLicense->setObjectName(QString::fromUtf8("b_RemoveSelectedFromLicense"));
        b_RemoveSelectedFromLicense->setGeometry(QRect(426, 150, 101, 23));
        CB_FeatureIdPicker = new QComboBox(LicenseGenerate);
        CB_FeatureIdPicker->setObjectName(QString::fromUtf8("CB_FeatureIdPicker"));
        CB_FeatureIdPicker->setGeometry(QRect(306, 110, 111, 22));
        t_OutputLicensePath = new QTextEdit(LicenseGenerate);
        t_OutputLicensePath->setObjectName(QString::fromUtf8("t_OutputLicensePath"));
        t_OutputLicensePath->setGeometry(QRect(86, 370, 331, 31));
        L_EndDate = new QLabel(LicenseGenerate);
        L_EndDate->setObjectName(QString::fromUtf8("L_EndDate"));
        L_EndDate->setGeometry(QRect(246, 70, 46, 13));
        L_ProductIdPicker = new QLabel(LicenseGenerate);
        L_ProductIdPicker->setObjectName(QString::fromUtf8("L_ProductIdPicker"));
        L_ProductIdPicker->setGeometry(QRect(16, 110, 51, 21));
        b_GenerateLicense = new QPushButton(LicenseGenerate);
        b_GenerateLicense->setObjectName(QString::fromUtf8("b_GenerateLicense"));
        b_GenerateLicense->setGeometry(QRect(430, 370, 91, 23));

        retranslateUi(LicenseGenerate);

        QMetaObject::connectSlotsByName(LicenseGenerate);
    } // setupUi

    void retranslateUi(QFrame *LicenseGenerate)
    {
        LicenseGenerate->setWindowTitle(QApplication::translate("LicenseGenerate", "Generate License", 0, QApplication::UnicodeUTF8));
        L_ClientSeed->setText(QApplication::translate("LicenseGenerate", "Client seed", 0, QApplication::UnicodeUTF8));
        L_StartDate->setText(QApplication::translate("LicenseGenerate", "Start Date", 0, QApplication::UnicodeUTF8));
        b_AddToLicense->setText(QApplication::translate("LicenseGenerate", "Add to license", 0, QApplication::UnicodeUTF8));
        L_FeatureIdPicker->setText(QApplication::translate("LicenseGenerate", "Feature", 0, QApplication::UnicodeUTF8));
        L_OutputFile->setText(QApplication::translate("LicenseGenerate", "Output file", 0, QApplication::UnicodeUTF8));
        b_BrowseClientSeed->setText(QApplication::translate("LicenseGenerate", "Browse", 0, QApplication::UnicodeUTF8));
        b_RemoveSelectedFromLicense->setText(QApplication::translate("LicenseGenerate", "Remove selected", 0, QApplication::UnicodeUTF8));
        L_EndDate->setText(QApplication::translate("LicenseGenerate", "End date", 0, QApplication::UnicodeUTF8));
        L_ProductIdPicker->setText(QApplication::translate("LicenseGenerate", "Product", 0, QApplication::UnicodeUTF8));
        b_GenerateLicense->setText(QApplication::translate("LicenseGenerate", "Generate license", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class LicenseGenerate: public Ui_LicenseGenerate {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_GENERATELICENSEUI_H
