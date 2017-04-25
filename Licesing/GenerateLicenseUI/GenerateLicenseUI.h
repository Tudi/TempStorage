#ifndef LICENSEGENERATE_H
#define LICENSEGENERATE_H
 
#include "ui_GenerateLicenseUI.h"
 
 
class myQtApp : public QFrame, private Ui::LicenseGenerate
{
    Q_OBJECT
 
public:
	myQtApp(QFrame *parent = 0);
 
 
public slots:
 /*   void getPath();
    void doSomething();
    void clear();
    void about(); */
};
 
 
#endif