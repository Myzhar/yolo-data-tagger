#include "qimagescene.h"

#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QTextDocument>

QImageScene::QImageScene(QObject *parent)
    : QGraphicsScene(parent)
    , mImgItem(NULL)
    , mCurrBBox(NULL)
{
    mDrawingEnabled = false;
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

        emit removedBBox( item );

        removeItem(item);
    }
}

void QImageScene::setImage(QImage &image)
{
    mImgItem->setPixmap( QPixmap::fromImage(image) );
    mImgItem->setPos( 0,0 );
    mImgItem->setZValue( -10.0 );
}

QSizeF QImageScene::getImageSize()
{
    return mImgItem->boundingRect().size();
}

void QImageScene::setBBoxLabel(QString label, QColor& color)
{
    mLabel = label;
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
        newBBox->setBrush( QBrush( QColor(mLabelColor.red(), mLabelColor.green(), mLabelColor.blue(), 100 ) ) );

        QRectF rect = QRectF(mouseScenePos,mouseScenePos).normalized();
        newBBox->setRect( rect );

        QGraphicsTextItem* labelItem = new QGraphicsTextItem( mLabel );
        labelItem->setParentItem( newBBox );
        labelItem->setPos( rect.topLeft() );
        labelItem->setDefaultTextColor( QColor( (255-mLabelColor.red())/2,(255-mLabelColor.green())/2,(255-mLabelColor.blue())/2,200 ));

        addItem( newBBox );

        mCurrBBox = newBBox;
    }
    else if( event->button() == Qt::RightButton )
    {
        QGraphicsItem* item = itemAt( pt, QTransform() );

        if( !item || item==mImgItem || dynamic_cast<QGraphicsTextItem*>(item))
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
        foreach (QGraphicsItem* item, mCurrBBox->childItems())
        {
            QGraphicsTextItem* txtItem =  dynamic_cast<QGraphicsTextItem*>(item);

            if(txtItem)
            {
                txtItem->setPos( rect.topLeft() );

                QFont font = txtItem->font();
                int fontSize = qMax(2,int(qAbs(rect.height())/4));
                font.setPixelSize( fontSize );
                txtItem->setFont( font );

                while( txtItem->boundingRect().width() > rect.width() )
                {
                    fontSize = font.pixelSize();

                    fontSize--;
                    font.setPixelSize( fontSize );
                    txtItem->setFont( font );

                    if( fontSize<2 )
                        break;
                }
            }
        }
    }
}

void QImageScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if( mCurrBBox==NULL )
        return;

    if( event->button() == Qt::LeftButton )
    {
        QRectF rect = mCurrBBox->rect();

        if( rect.width()<5 || rect.height()<5)
        {
            mCurrBBox = NULL;
            return;
        }

        QRectF scRect = sceneRect();

        qreal normX = (rect.topLeft().x()/scRect.width());
        qreal normY = (rect.topLeft().y()/scRect.height());

        qreal normW = (rect.width()/scRect.width());
        qreal normH = (rect.height()/scRect.height());

        emit newBBox( mCurrBBox, normX, normY, normW, normH );

        mCurrBBox = NULL;
    }
}

QGraphicsItem* QImageScene::addBBox( QString label, QColor& color, double nx, double ny, double nw, double nh )
{
    QRectF scRect = sceneRect();

    if( scRect.width()==0 || scRect.height()==0 )
        return NULL;

    QGraphicsRectItem* newBox = new QGraphicsRectItem( );

    newBox->setRect( nx*scRect.width(), ny*scRect.height(),
                     nw*scRect.width(), nh*scRect.height() );

    addItem( newBox );

    QPen pen( color );
    pen.setWidth(5);

    newBox->setPen( pen );
    newBox->setBrush( QBrush( QColor(color.red(), color.green(), color.blue(), 100 ) ) );

    QRectF rect = newBox->rect();

    QGraphicsTextItem* labelItem = new QGraphicsTextItem( label );
    labelItem->setParentItem( newBox );
    labelItem->setPos( rect.topLeft() );
    labelItem->setDefaultTextColor( QColor( (255-color.red())/2,(255-color.green())/2,(255-color.blue())/2,200 ));

    QFont font = labelItem->font();
    int fontSize = qMax(2,int(qAbs(rect.height())/4));
    font.setPixelSize( fontSize );
    labelItem->setFont( font );

    while( labelItem->boundingRect().width() > rect.width() )
    {
        fontSize = font.pixelSize();

        fontSize--;
        font.setPixelSize( fontSize );
        labelItem->setFont( font );

        if( fontSize<2 )
            break;
    }

    emit newBBox( newBox, nx, ny, nw, nh );

    return newBox;
}
