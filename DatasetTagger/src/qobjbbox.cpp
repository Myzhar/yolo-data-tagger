#include "include/qobjbbox.h"

#include <QTextStream>
#include <QLocale>

QObjBBox::QObjBBox(int labIdx, double nx, double ny, double nw, double nh)
    : QObject(NULL)
{
    mLabelIdx = labIdx;
    mNormX = nx;
    mNormY = ny;
    mNormW = nw;
    mNormH = nh;
}

QObjBBox::QObjBBox( const QObjBBox& other )
    : QObject(NULL)
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

QByteArray QObjBBox::getYoloTsLine()
{
    QLocale locale( "C" );
    QLocale::setDefault( locale );

    QByteArray res;
    QTextStream str( res );
    str.setRealNumberNotation(QTextStream::SmartNotation);
    str.setRealNumberPrecision(10);

    str << mLabelIdx << " ";
    str << mNormX << " ";
    str << mNormY << " ";
    str << mNormW << " ";
    str << mNormH << "\n";

    return res;
}
