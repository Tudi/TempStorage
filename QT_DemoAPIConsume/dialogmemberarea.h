#ifndef DIALOGMEMBERAREA_H
#define DIALOGMEMBERAREA_H

#include <QDialog>

namespace Ui {
class DialogMemberArea;
}

class DialogMemberArea : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMemberArea(QWidget *parent = nullptr);
    ~DialogMemberArea();
    void resizeEvent(QResizeEvent *event);
private:
    QWidget* scrollAreaContent;
    Ui::DialogMemberArea *ui;
};

#endif // DIALOGMEMBERAREA_H
