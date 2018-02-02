#include "include/qtrainsetexample.h"

#include <QDebug>
#include <iostream>

#include <QDir>
#include <QHashIterator>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QRandomGenerator>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

QTrainSetExample::QTrainSetExample(QString imgFilename, QObject *parent) : QObject(parent)
{
    mImageFilename = imgFilename;
    mTestSet = false;
}

void QTrainSetExample::setRelFolderPath( QString fullPath, QString basePath )
{
    mFullFolderPath = fullPath;

    QDir baseDir(basePath);

    mRelativeFolderPath = baseDir.relativeFilePath( fullPath );

}

void QTrainSetExample::addNewBBox( quint64 bboxIdx, int labIdx, double nx, double ny, double nw, double nh )
{
    if( (nx-nw/2)<0.0 || (ny-nh/2)<0.0 ||
            nw<=0.0 || nh<=0.0 ||
            (nx+nw/2)>=1.0 || (ny+nh/2)>=1.0 ||
            nw>1.0 || nh>1.0)
    {
        return;
    }

    QObjBBox* bbox = new QObjBBox(labIdx,nx,ny,nw,nh);

    mBboxMap[bboxIdx] = bbox;
}

void QTrainSetExample::removeBBox(quint64 bboxIdx )
{
    mBboxMap.remove( bboxIdx );
}

void QTrainSetExample::setFullFolderPath( QString fullPath )
{
    mFullFolderPath = fullPath;
}

QString QTrainSetExample::getFullPath()
{
    return mFullFolderPath;
}

QString QTrainSetExample::getRelPath()
{
    return mRelativeFolderPath;
}

QString QTrainSetExample::getImgName()
{
    return mImageFilename;
}

void QTrainSetExample::setTestSet(bool testSet)
{
    mTestSet = testSet;
}

bool QTrainSetExample::isTestSample()
{
    return mTestSet;
}

const QHash<quint64,QObjBBox*>& QTrainSetExample::getBBoxes()
{
    return mBboxMap;
}

QStringList QTrainSetExample::getBboxesStrings()
{
    if( mBboxMap.size()==0 )
        return QStringList();

    QStringList bboxes;

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        QString bboxStr = bbox->getYoloBBoxLine();

        bboxStr.chop(1); // remove '/n'

        bboxes.push_back( bboxStr);
    }

    return bboxes;
}

bool QTrainSetExample::saveYoloFormat()
{
    QString filename = mFullFolderPath;
    if(!filename.endsWith("/"))
    {
        filename += "/";
    }

    filename += mImageFilename;

    // >>>>> Remove extension
    int dotIdx = filename.lastIndexOf( "." );
    filename.truncate( dotIdx+1 );
    // <<<<< Remove extension

    filename += "txt";

    QFile bboxFile(filename);

    if( !bboxFile.open( QFile::WriteOnly|QFile::Text ) )
    {
        return false;
    }

    QTextStream stream( &bboxFile );

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        QString bboxStr = bbox->getYoloBBoxLine();

        stream << bboxStr;
    }

    return true;
}

bool QTrainSetExample::loadYoloFormat()
{
    QString filename = mFullFolderPath;
    if(!filename.endsWith("/"))
    {
        filename += "/";
    }

    filename += mImageFilename;

    // >>>>> Remove extension
    int dotIdx = filename.lastIndexOf( "." );
    filename.truncate( dotIdx+1 );
    // <<<<< Remove extension

    filename += "txt";

    QFile bboxFile(filename);

    if( !bboxFile.open( QFile::ReadOnly|QFile::Text ) )
    {
        return false;
    }

    QTextStream stream( &bboxFile );

    int lineIdx = -1;
    int bboxIdx = 0;

    while( !stream.atEnd() )
    {
        QString line = stream.readLine();
        lineIdx++;
        QStringList tokens = line.split( " " );

        if( tokens.size() != 5 )
        {
            qDebug() << tr("Wrong line %1:[%2]in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        int labelIdx;
        double normX;
        double normY;
        double normW;
        double normH;

        bool ok;

        labelIdx = tokens.at(0).toInt( &ok );
        if( !ok )
        {
            qDebug() << tr("Wrong 'labelIdx' in line %1:[%2] in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        normX = tokens.at(1).toDouble( &ok );
        if( !ok || normX<0.0 || normX>1.0 )
        {
            qDebug() << tr("Wrong 'normX' in line %1:[%2] in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        normY = tokens.at(2).toDouble( &ok );
        if( !ok || normY<0.0 || normY>1.0 )
        {
            qDebug() << tr("Wrong 'normY' in line %1:[%2] in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        normW = tokens.at(3).toDouble( &ok );
        if( !ok || normW<0.0 || normW>1.0 )
        {
            qDebug() << tr("Wrong 'normW' in line %1:[%2] in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        normH = tokens.at(4).toDouble( &ok );
        if( !ok || normH<0.0 || normH>1.0 )
        {
            qDebug() << tr("Wrong 'normW' in line %1:[%2] in file %3").arg(lineIdx).arg(line).arg(filename);
            continue;
        }

        QObjBBox* bbox = new QObjBBox(labelIdx,normX,normY,normW,normH);

        mBboxMap[bboxIdx] = bbox;
        bboxIdx++;
    }

    /*QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        QString bboxStr = bbox->getYoloBBoxLine();

        stream << bboxStr;
    }*/

    return true;
}

QTrainSetExample* QTrainSetExample::cloneBlur()
{
    // >>>>> 1) Open sample image
    QString origFullPathName = mFullFolderPath;
    if( !origFullPathName.endsWith("/"))
        origFullPathName += "/";

    origFullPathName += mImageFilename;
    cv::Mat orig = cv::imread( origFullPathName.toStdString() );

    if( orig.empty() )
    {
        qDebug() << tr("Error generation Blurred image: %1 not opened").arg(mFullFolderPath);
        return nullptr;
    }
    // <<<<< 1) Open sample image

    // >>>>> 2) Apply BLUR
    cv::Mat blurred;

    cv::blur( orig, blurred, cv::Size(7,7) );
    cv::blur( blurred, blurred, cv::Size(5,5) );
    // <<<<< 2) Apply BLUR

    // >>>>> 3) Save image and create new Dataset sample updating Bounding Boxes
    QString newImageFilename = mImageFilename;
    int dotIdx = newImageFilename.lastIndexOf( "." );
    newImageFilename.insert( dotIdx, "_B" );

    QTrainSetExample* newTs = new QTrainSetExample(newImageFilename);

    newTs->mFullFolderPath = mFullFolderPath;
    newTs->mRelativeFolderPath = mRelativeFolderPath;

    QString fullPathName = mFullFolderPath;
    if( !fullPathName.endsWith("/"))
        fullPathName += "/";

    fullPathName += newImageFilename;

    cv::imwrite( fullPathName.toStdString(), blurred );

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    int idx = 0;
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        if( bbox->mNormX<0.0 || bbox->mNormY<0.0 ||
                bbox->mNormW<=0.0 || bbox->mNormH<=0.0 ||
                bbox->mNormX>=1.0 || bbox->mNormY>=1.0 ||
                bbox->mNormW>1.0 || bbox->mNormH>1.0)
        {
            continue;
        }

        newTs->addNewBBox( ++idx, bbox->mLabelIdx, bbox->mNormX, bbox->mNormY, bbox->mNormW, bbox->mNormH );
    }
    // <<<<< 3) Save image and create new Dataset sample updating Bounding Boxes

    return newTs;
}

QTrainSetExample* QTrainSetExample::cloneSaltAndPepper()
{
    // >>>>> 1) Open sample image
    QString origFullPathName = mFullFolderPath;
    if( !origFullPathName.endsWith("/"))
        origFullPathName += "/";

    origFullPathName += mImageFilename;
    cv::Mat orig = cv::imread( origFullPathName.toStdString() );

    if( orig.empty() )
    {
        qDebug() << tr("Error generation Blurred image: %1 not opened").arg(mFullFolderPath);
        return nullptr;
    }
    // <<<<< 1) Open sample image

    // >>>>> 2) Add Salt and Pepper noise
    cv::Mat noised;
    orig.copyTo(noised);

    double salt_vs_pepper = 0.2;
    double amount = 0.004;

    int num_salt = static_cast<int>( ceil(amount * noised.size().area() * salt_vs_pepper) );
    int num_pepper = static_cast<int>( ceil(amount * noised.size().area() * (1.0 - salt_vs_pepper)) );

    QRandomGenerator rnd;

    cv::Vec3b salt(255,255,255);

    for( int s=0; s<num_salt; s++ )
    {
        quint32 x = rnd.bounded(noised.cols);
        quint32 y = rnd.bounded(noised.rows);

        noised.at<cv::Vec3b>(y,x) =salt;
    }

    cv::Vec3b pepper(0,0,0);
    for( int s=0; s<num_pepper; s++ )
    {
        quint32 x = rnd.bounded(noised.cols);
        quint32 y = rnd.bounded(noised.rows);

        noised.at<cv::Vec3b>(y,x)=pepper;
    }
    // <<<<< 2) Add Salt and Pepper noise

    // >>>>> 3) Save image and create new Dataset sample updating Bounding Boxes
    QString newImageFilename = mImageFilename;
    int dotIdx = newImageFilename.lastIndexOf( "." );
    newImageFilename.insert( dotIdx, "_SP" );
    QTrainSetExample* newTs = new QTrainSetExample(newImageFilename);

    newTs->mFullFolderPath = mFullFolderPath;
    newTs->mRelativeFolderPath = mRelativeFolderPath;

    QString fullPathName = mFullFolderPath;
    if( !fullPathName.endsWith("/"))
        fullPathName += "/";

    fullPathName += newImageFilename;

    cv::imwrite( fullPathName.toStdString(), noised );

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    int idx = 0;
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        if( bbox->mNormX<0.0 || bbox->mNormY<0.0 ||
                bbox->mNormW<=0.0 || bbox->mNormH<=0.0 ||
                bbox->mNormX>=1.0 || bbox->mNormY>=1.0 ||
                bbox->mNormW>1.0 || bbox->mNormH>1.0)
        {
            continue;
        }

        newTs->addNewBBox( ++idx, bbox->mLabelIdx, bbox->mNormX, bbox->mNormY, bbox->mNormW, bbox->mNormH );
    }
    // <<<<< 3) Save image and create new Dataset sample updating Bounding Boxes

    return newTs;
}

QTrainSetExample* QTrainSetExample::cloneFlip( int mode )
{
    // >>>>> 1) Open sample image
    QString origFullPathName = mFullFolderPath;
    if( !origFullPathName.endsWith("/"))
        origFullPathName += "/";

    origFullPathName += mImageFilename;
    cv::Mat orig = cv::imread( origFullPathName.toStdString() );

    if( orig.empty() )
    {
        qDebug() << tr("Error generation Blurred image: %1 not opened").arg(mFullFolderPath);
        return nullptr;
    }
    // <<<<< 1) Open sample image

    // >>>>> 2) Flip Image
    cv::Mat flipped;

    if( mode==0 )
    {
        cv::flip( orig, flipped, 0 );
    }
    else
    {
        cv::flip( orig, flipped, 1 );
    }

    // <<<<< 2) Flip Image

    // >>>>> 3) Save image and create new Dataset sample updating Bounding Boxes
    QString newImageFilename = mImageFilename;
    int dotIdx = newImageFilename.lastIndexOf( "." );

    if( mode == 0)
    {
        newImageFilename.insert( dotIdx, "_FV" );
    }
    else
    {
        newImageFilename.insert( dotIdx, "_FH" );
    }

    QTrainSetExample* newTs = new QTrainSetExample(newImageFilename);

    newTs->mFullFolderPath = mFullFolderPath;
    newTs->mRelativeFolderPath = mRelativeFolderPath;

    QString fullPathName = mFullFolderPath;
    if( !fullPathName.endsWith("/"))
        fullPathName += "/";

    fullPathName += newImageFilename;

    cv::imwrite( fullPathName.toStdString(), flipped );

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    int idx = 0;
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        // >>>>> Flip BBox
        double nx,ny;

        if( mode==0 )
        {
            nx = bbox->mNormX;
            ny = 0.5 + (0.5-bbox->mNormY) - bbox->mNormH;
        }
        else
        {
            nx = 0.5 + (0.5-bbox->mNormX) - bbox->mNormW;
            ny = bbox->mNormY;
        }
        // <<<<< Flip BBox

        if( bbox->mNormX<0.0 || bbox->mNormY<0.0 ||
                bbox->mNormW<=0.0 || bbox->mNormH<=0.0 ||
                bbox->mNormX>=1.0 || bbox->mNormY>=1.0 ||
                bbox->mNormW>1.0 || bbox->mNormH>1.0)
        {
            continue;
        }

        newTs->addNewBBox( ++idx, bbox->mLabelIdx, nx, ny, bbox->mNormW, bbox->mNormH );
    }
    // <<<<< 4) Create new Dataset sample updating Bounding Boxes

    return newTs;
}

QObjBBox QTrainSetExample::rotateBBox( QObjBBox* bbox,  cv::Size imgSize, cv::Mat RT )
{
    double x0 = bbox->mNormX*imgSize.width;
    double y0 = bbox->mNormY*imgSize.height;

    double x1 = x0+bbox->mNormW*imgSize.width;
    double y1 = y0+bbox->mNormH*imgSize.height;

    cv::Mat RTN(3,3,CV_64F);
    RTN.at<double>(0,0) = RT.at<double>(0,0);
    RTN.at<double>(0,1) = RT.at<double>(0,1);
    RTN.at<double>(0,2) = RT.at<double>(0,2);
    RTN.at<double>(1,0) = RT.at<double>(1,0);
    RTN.at<double>(1,1) = RT.at<double>(1,1);
    RTN.at<double>(1,2) = RT.at<double>(1,2);
    RTN.at<double>(2,0) = 0.0;
    RTN.at<double>(2,1) = 0.0;
    RTN.at<double>(2,2) = 1.0;

    cv::Mat tl(3,1,CV_64F);
    tl.at<double>(0,0) = x0;
    tl.at<double>(1,0) = y0;
    tl.at<double>(2,0) = 1.0;

    cv::Mat tr(3,1,CV_64F);
    tr.at<double>(0,0) = x1;
    tr.at<double>(1,0) = y0;
    tr.at<double>(2,0) = 1.0;

    cv::Mat br(3,1,CV_64F);
    br.at<double>(0,0) = x1;
    br.at<double>(1,0) = y1;
    br.at<double>(2,0) = 1.0;

    cv::Mat bl(3,1,CV_64F);
    bl.at<double>(0,0) = x0;
    bl.at<double>(1,0) = y1;
    bl.at<double>(2,0) = 1.0;

    cv::Mat tlR;
    cv::Mat trR;
    cv::Mat brR;
    cv::Mat blR;

    tlR = RTN*tl;
    trR = RTN*tr;
    brR = RTN*br;
    blR = RTN*bl;

    int minX = 10000;
    int maxX = 0;
    int minY = 10000;
    int maxY = 0;

    cv::Point rTl;
    rTl.x = static_cast<int>(tlR.at<double>(0,0));
    rTl.y = static_cast<int>(tlR.at<double>(1,0));
    minX = rTl.x;
    maxX = rTl.x;
    minY = rTl.y;
    maxY = rTl.y;

    cv::Point rTr;
    rTr.x = static_cast<int>(trR.at<double>(0,0));
    rTr.y = static_cast<int>(trR.at<double>(1,0));
    minX = qMin(minX,rTr.x);
    maxX = qMax(maxX,rTr.x);
    minY = qMin(minY,rTr.y);
    maxY = qMax(maxY,rTr.y);

    cv::Point rBr;
    rBr.x = static_cast<int>(brR.at<double>(0,0));
    rBr.y = static_cast<int>(brR.at<double>(1,0));
    minX = qMin(minX,rBr.x);
    maxX = qMax(maxX,rBr.x);
    minY = qMin(minY,rBr.y);
    maxY = qMax(maxY,rBr.y);

    cv::Point rBl;
    rBl.x = static_cast<int>(blR.at<double>(0,0));
    rBl.y = static_cast<int>(blR.at<double>(1,0));
    minX = qMin(minX,rBl.x);
    maxX = qMax(maxX,rBl.x);
    minY = qMin(minY,rBl.y);
    maxY = qMax(maxY,rBl.y);

    minX = qMax(0,minX);
    minX = qMin(imgSize.width,minX);
    maxX = qMin(imgSize.width,maxX);
    maxX = qMax(0,maxX);
    minY = qMax(0,minY);
    minY = qMin(imgSize.height,minY);
    maxY = qMin(imgSize.height,maxY);
    maxY = qMax(0,maxY);

    QObjBBox newBBox;
    newBBox.mNormX = static_cast<double>(minX)/imgSize.width;
    newBBox.mNormY = static_cast<double>(minY)/imgSize.height;;
    newBBox.mNormW = static_cast<double>(maxX-minX)/imgSize.width;
    newBBox.mNormH = static_cast<double>(maxY-minY)/imgSize.height;

    return newBBox;
}

QTrainSetExample* QTrainSetExample::cloneRotateScale( double angleDeg, double scale )
{
    // >>>>> 1) Open sample image
    QString origFullPathName = mFullFolderPath;
    if( !origFullPathName.endsWith("/"))
        origFullPathName += "/";

    origFullPathName += mImageFilename;
    cv::Mat orig = cv::imread( origFullPathName.toStdString() );

    if( orig.empty() )
    {
        qDebug() << tr("Error generation Blurred image: %1 not opened").arg(mFullFolderPath);
        return nullptr;
    }
    // <<<<< 1) Open sample image

    // >>>>> 2) Rotate Image
    cv::Mat rotated;

    cv::Point2f rotCenter(orig.cols/2., orig.rows/2.);
    cv::Mat rotMat = cv::getRotationMatrix2D( rotCenter, angleDeg, scale );
    cv::warpAffine(orig, rotated, rotMat, orig.size());

    // <<<<< 2) Rotate Image

    // >>>>> 3) Save image and create new Dataset sample updating Bounding Boxes
    QString newImageFilename = mImageFilename;
    int dotIdx = newImageFilename.lastIndexOf( "." );

    QString suffix = tr("_%1").arg( angleDeg, 3, 'g', 3, QChar('0') );
    newImageFilename.insert( dotIdx, suffix );

    QTrainSetExample* newTs = new QTrainSetExample(newImageFilename);

    newTs->mFullFolderPath = mFullFolderPath;
    newTs->mRelativeFolderPath = mRelativeFolderPath;

    QString fullPathName = mFullFolderPath;
    if( !fullPathName.endsWith("/"))
        fullPathName += "/";

    fullPathName += newImageFilename;

    cv::imwrite( fullPathName.toStdString(), rotated );

    QHashIterator<quint64,QObjBBox*> iter(mBboxMap);
    int idx = 0;
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        if( bbox->mNormX<0.0 || bbox->mNormY<0.0 ||
                bbox->mNormW<=0.0 || bbox->mNormH<=0.0 ||
                bbox->mNormX>=1.0 || bbox->mNormY>=1.0 ||
                bbox->mNormW>1.0 || bbox->mNormH>1.0)
        {
            continue;
        }

        // >>>>> Rotate BBox
        QObjBBox rotBbox = rotateBBox( bbox, orig.size(), rotMat );

        // <<<<< Rotate BBox

        newTs->addNewBBox( ++idx, bbox->mLabelIdx, rotBbox.mNormX, rotBbox.mNormY, rotBbox.mNormW, rotBbox.mNormH );
    }
    // <<<<< 4) Create new Dataset sample updating Bounding Boxes

    return newTs;
}
