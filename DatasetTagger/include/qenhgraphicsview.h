#ifndef QENHGRAPHICSVIEW_H
#define QENHGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>


class QEnhGraphicsView : public QGraphicsView
{

public:
    QEnhGraphicsView(QWidget *parent = Q_NULLPTR);
    virtual ~QEnhGraphicsView();

protected:
    void wheelEvent ( QWheelEvent * event );

protected slots:
    void scalingTime(qreal x);
    void animFinished();

private:
    int mNumScheduledScalings;
};

#endif // QENHGRAPHICSVIEW_H
