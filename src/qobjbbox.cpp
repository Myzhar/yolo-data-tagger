#include "include/qobjbbox.h"

#include <QTextStream>
#include <QLocale>

QObjBBox::QObjBBox(int labIdx, double nx, double ny, double nw, double nh)
    : QObject(nullptr)
{
    setBBox(labIdx,nx,ny,nw,nh);
}

QObjBBox::QObjBBox( const QObjBBox& other )
    : QObject(nullptr)
{
    mLabelIdx = other.mLabelIdx;
    mNormX = other.mNormX;
    mNormY = other.mNormY;
    mNormW = other.mNormW;
    mNormH = other.mNormH;
}

QObjBBox& QObjBBox::operator=( const QObjBBox& other)
{
    if(&other != this)
    {
        mLabelIdx = other.mLabelIdx;
        mNormX = other.mNormX;
        mNormY = other.mNormY;
        mNormW = other.mNormW;
        mNormH = other.mNormH;
    }

    return *this;
}

void QObjBBox::setBBox( int labIdx, double nx, double ny, double nw, double nh )
{
    mLabelIdx = labIdx;
    mNormX = nx;
    mNormY = ny;
    mNormW = nw;
    mNormH = nh;
}

void QObjBBox::getBBox( int& labIdx, double& nx, double& ny, double& nw, double& nh )
{
    labIdx = mLabelIdx;
    nx = mNormX;
    ny = mNormY;
    nw = mNormW;
    nh = mNormH;
}

QString QObjBBox::getYoloBBoxLine()
{
    QLocale locale( "C" );
    QLocale::setDefault( locale );

    QString res;
    QTextStream str( &res, QIODevice::ReadWrite );
    str.setRealNumberNotation(QTextStream::SmartNotation);
    str.setRealNumberPrecision(10);

    str << mLabelIdx << " ";
    str << mNormX << " ";
    str << mNormY << " ";
    str << mNormW << " ";
    str << mNormH << "\n";

    return res;
}
