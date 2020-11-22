#ifndef DIALOGEDITPROJECT_H
#define DIALOGEDITPROJECT_H

#include <QDialog>

namespace Ui {
class DialogEditProject;
}

class FrameProjectDetailsListView;

class DialogEditProject : public QDialog
{
    Q_OBJECT

public:
    explicit DialogEditProject(QWidget *parent = nullptr);
    ~DialogEditProject();
    void SetActive(int NewState);
    void SetProjectName(QString pName);
    void SetProjectId(int pProjectId);
    void SetOwner(FrameProjectDetailsListView *pOwner);
private slots:
    void on_b_ChangeName_clicked();

    void on_cb_Active_stateChanged(int arg1);

private:
    FrameProjectDetailsListView *owner;
    int ProjectId;
    Ui::DialogEditProject *ui;
};

#endif // DIALOGEDITPROJECT_H
