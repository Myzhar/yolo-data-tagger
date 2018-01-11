#ifndef QOBJBBOX_H
#define QOBJBBOX_H

#include <QObject>

class QObjBBox : public QObject
{
    Q_OBJECT
public:
    explicit QObjBBox( int labIdx=-1, double nx=0.0, double ny=0.0, double nw=0.0, double nh=0.0 );
    QObjBBox( const QObjBBox& other );
    QObjBBox& operator=( const QObjBBox& other );

    void setBBox( int labIdx, double nx, double ny, double nw, double nh );
    void getBBox( int& labIdx, double& nx, double& ny, double& nw, double& nh );

    QString getYoloBBoxLine();

signals:

public:
    int mLabelIdx;
    double mNormX;
    double mNormY;
    double mNormW;
    double mNormH;
};

#endif // QOBJBBOX_H
