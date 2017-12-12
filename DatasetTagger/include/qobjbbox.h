#ifndef QOBJBBOX_H
#define QOBJBBOX_H

#include <QObject>

class QObjBBox : public QObject
{
    Q_OBJECT
public:
    explicit QObjBBox( int labIdx, double nx, double ny, double nw, double nh );
    QObjBBox( const QObjBBox& other );
    QObjBBox& operator=( const QObjBBox& other );

    QByteArray getYoloTsLine();

signals:

public:
    int mLabelIdx;
    double mNormX;
    double mNormY;
    double mNormW;
    double mNormH;
};

#endif // QOBJBBOX_H
