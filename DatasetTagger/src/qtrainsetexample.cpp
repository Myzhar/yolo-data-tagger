#include "include/qtrainsetexample.h"

#include <QDebug>
#include <QDir>
#include <QHashIterator>
#include <QTextStream>
#include <QString>
#include <QStringList>

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

void QTrainSetExample::addBBox( quint64 bboxIdx, int labIdx, double nx, double ny, double nw, double nh )
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
