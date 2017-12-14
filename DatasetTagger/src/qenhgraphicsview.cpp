#include "qenhgraphicsview.h"
#include <QTimeLine>
#include <QDebug>

QEnhGraphicsView::QEnhGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    mNumScheduledScalings = 0;
}

QEnhGraphicsView::~QEnhGraphicsView()
{

}

void QEnhGraphicsView::wheelEvent ( QWheelEvent * event )
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15; // see QWheelEvent documentation
    mNumScheduledScalings += numSteps;
    if (mNumScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
        mNumScheduledScalings = numSteps;

    QTimeLine *anim = new QTimeLine(350, this);
    anim->setUpdateInterval(10);

    connect(anim, &QTimeLine::valueChanged,
            this, &QEnhGraphicsView::scalingTime );
    connect(anim, &QTimeLine::finished,
            this, &QEnhGraphicsView::animFinished );

    anim->start();

    centerOn( mapToScene(event->pos()) );
}

void QEnhGraphicsView::scalingTime(qreal x)
{
    qreal factor = 1.0 + qreal(mNumScheduledScalings) / 300.0;

    scale(factor, factor);
}

void QEnhGraphicsView::animFinished()
{
    if (mNumScheduledScalings > 0)
        mNumScheduledScalings--;
    else
        mNumScheduledScalings++;

    sender()->~QObject();
}
