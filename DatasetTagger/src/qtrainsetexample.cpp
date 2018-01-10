#include "include/qtrainsetexample.h"

#include <QDir>
#include <QHashIterator>


QTrainSetExample::QTrainSetExample(QString imgFilename, QObject *parent) : QObject(parent)
{
    mImageFilename = imgFilename;
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

        QString bboxStr = bbox->getYoloTsLine();

        bboxStr.chop(1); // remove '/n'

        bboxes.push_back( bboxStr);
    }

    return bboxes;
}
