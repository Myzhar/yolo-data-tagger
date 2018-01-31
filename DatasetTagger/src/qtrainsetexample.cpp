#include "include/qtrainsetexample.h"

#include <QDebug>
#include <QDir>
#include <QHashIterator>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QRandomGenerator>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

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

        newTs->addNewBBox( ++idx, bbox->mLabelIdx, nx, ny, bbox->mNormW, bbox->mNormH );
    }
    // <<<<< 4) Create new Dataset sample updating Bounding Boxes

    return newTs;
}
