#ifndef QIMAGESCENE_H
#define QIMAGESCENE_H

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QImage>

class QImageScene : public QGraphicsScene
{
    Q_OBJECT

public:
    QImageScene(QObject *parent = 0);
    virtual ~QImageScene();

    void setImage( QImage& image );
    QSizeF getImageSize();
    void setBBoxLabel( QString label, QColor& color );

    void enableDrawing(bool enable);
    void removeAllBBoxes();

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

signals:
    void newBBox(QGraphicsItem* item, double nx, double ny, double nw, double nh);
    void removedBBox( QGraphicsItem* item );

private:

    QGraphicsPixmapItem* mImgItem;

    QGraphicsRectItem* mCurrBBox;

    QColor mLabelColor;
    QString mLabel;

    bool mDrawingEnabled;
};

#endif // QIMAGESCENE_H
