#ifndef FRAMEPROJECTDETAILSLISTVIEW_H
#define FRAMEPROJECTDETAILSLISTVIEW_H

#include <QFrame>

namespace Ui {
class FrameProjectDetailsListView;
}

class QNetworkReply;

class FrameProjectDetailsListView : public QFrame
{
    Q_OBJECT
    Q_SLOT void slot_netwManagerFinished(QNetworkReply *reply);
public:
    explicit FrameProjectDetailsListView(QWidget *parent = nullptr);
    ~FrameProjectDetailsListView();
    void SetName(QString name);
    void SetIcon(QString name);
    void SetActive(bool);
    void SetTimeWeek(int Seconds);
    void SetTimeMonth(int Seconds);
    void SetTimeTotal(int Seconds);
    void SetProjectId(int pProjectId);
private:
    int ProjectId;
    bool Active;
    void mouseReleaseEvent(QMouseEvent * event);
    Ui::FrameProjectDetailsListView *ui;
};

#endif // FRAMEPROJECTDETAILSLISTVIEW_H
