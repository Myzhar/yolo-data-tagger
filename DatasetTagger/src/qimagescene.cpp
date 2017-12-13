#include "qimagescene.h"

#include <QGraphicsSceneMouseEvent>

QImageScene::QImageScene(QObject *parent)
    : QGraphicsScene(parent)
    , mImgItem(NULL)
    , mCurrBBox(NULL)
{
    mImgItem = new QGraphicsPixmapItem();
    addItem( mImgItem );
}

QImageScene::~QImageScene()
{
    if(mImgItem)
    {
        delete mImgItem;
    }
}

void QImageScene::removeAllBBoxes()
{
    foreach( QGraphicsItem* item, items() )
    {
        if( dynamic_cast<QGraphicsPixmapItem*>(item) )
        {
            continue;
        }

        removeItem(item);
    }
}

void QImageScene::setImage(QImage &image)
{
    mImgItem->setPixmap( QPixmap::fromImage(image) );
    mImgItem->setPos( 0,0 );
    mImgItem->setZValue( -10.0 );


}

void QImageScene::setLabelColor( QColor& color )
{
    mLabelColor = color;
    mDrawingEnabled = true;
}

void QImageScene::enableDrawing(bool enable)
{
    mDrawingEnabled = enable;
}

void QImageScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF pt = event->scenePos();

    if( pt.x()<0 || pt.y()<0 )
        return;

    if( event->button() == Qt::LeftButton )
    {
        if(!mDrawingEnabled)
        {
            return;
        }

        QPointF mouseScenePos = event->buttonDownScenePos( Qt::MouseButton::LeftButton );

        //QRectF scRect = sceneRect();

        QGraphicsRectItem* newBBox = new QGraphicsRectItem( );

        QPen pen( mLabelColor );
        pen.setWidth(5);

        newBBox->setPen( pen );
        newBBox->setBrush( QBrush( QColor(mLabelColor.red(), mLabelColor.green(), mLabelColor.blue(), 50 ) ) );

        QRectF rect = QRectF(mouseScenePos,mouseScenePos).normalized();
        newBBox->setRect( rect );

        addItem( newBBox );

        mCurrBBox = newBBox;
    }
    else if( event->button() == Qt::RightButton )
    {
        QGraphicsItem* item = itemAt( pt, QTransform() );

        if(!item || item==mImgItem )
            return;

        emit removedBBox( item );

        removeItem( item );
    }
}

void QImageScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if( mCurrBBox==NULL )
        return;

    if( event->buttons() == Qt::LeftButton )
    {
        QPointF mouseDwScenePos = event->buttonDownScenePos( Qt::MouseButton::LeftButton );
        QPointF mousePos = event->scenePos();

        QRectF rect = QRectF(mouseDwScenePos,mousePos).normalized();

        if( rect.x() < 0 )
            rect.setX(0);

        if( rect.y() < 0 )
            rect.setY(0);

        QRectF imgRect = mImgItem->boundingRect();

        if( rect.right() > imgRect.width()-1 )
            rect.setRight(imgRect.width()-1);

        if( rect.bottom() > imgRect.height()-1 )
            rect.setBottom(imgRect.height()-1);

        mCurrBBox->setRect( rect );
    }
}

void QImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if( mCurrBBox==NULL )
        return;

    if( event->button() == Qt::LeftButton )
    {
        QRectF rect = mCurrBBox->rect();

        QRectF scRect = sceneRect();

        qreal normX = (rect.topLeft().x()/scRect.width());
        qreal normY = (rect.topLeft().y()/scRect.height());

        qreal normW = (rect.width()/scRect.width());
        qreal normH = (rect.height()/scRect.height());

        emit newBBox( mCurrBBox, normX, normY, normW, normH );

        mCurrBBox = NULL;
    }
}
