#include <QtGui> 
#include "GenerateLicenseUI.h"
 
// if we include <QtGui> there is no need to include every class used: <QString>, <QFileDialog>,...
 
myQtApp::myQtApp(QFrame *parent)
{
    setupUi(this); // this sets up GUI
 
    // signals/slots mechanism in action
//    connect( pushButton_browse, SIGNAL( clicked() ), this, SLOT( getPath() ) ); 
//    connect( pushButton_do, SIGNAL( clicked() ), this, SLOT( doSomething() ) ); 
//    connect( pushButton_clear, SIGNAL( clicked() ), this, SLOT( clear() ) ); 
//    connect( pushButton_about, SIGNAL( clicked() ), this, SLOT( about() ) ); 
}
/*
 
void myQtApp::getPath()
{
    QString path;
    
    path = QFileDialog::getOpenFileName(
        this,
        "Choose a file to open",
        QString::null,
        QString::null);
 
    lineEdit->setText( path );
}
 
 
void myQtApp::doSomething()
{
    int value1, value2;
    Qt::CheckState state;
    QString str;
 
    textEdit->append( "Path to file: " + lineEdit->text() );
 
    value1 = spinBox1->value();
    value2 = spinBox2->value();
 
    textEdit->append( "Number 1 value: " + QString::number(value1) );
    textEdit->append( "Number 2 value: " + QString::number(value2) );
 
    state = checkBox->checkState();
 
    str = "Checkbox says: ";
    if ( state == Qt::Checked ) str += "yes"; 
    else str += "no";
    textEdit->append( str );
 
    textEdit->append( "ComboBox current text: " + comboBox->currentText() );
    textEdit->append( "ComboBox current item: " + QString::number(comboBox->currentIndex()) );
}
 
 
void myQtApp::clear()
{
    textEdit->clear();
}
 
 
void myQtApp::about() 
{
    QMessageBox::about(this,"About myQtApp",
                "This app was coded for educational purposes.\n"
                "Number 1 is: " + QString::number(spinBox1->value()) + "\n\n"
                "Bye.\n");
}
*/