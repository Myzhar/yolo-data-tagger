#include "include/qtrainsetexample.h"

#include <QDir>


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

void QTrainSetExample::addBBox( int labIdx, double nx, double ny, double nw, double nh )
{
    QObjBBox bbox(labIdx,nx,ny,nw,nh);

    mBboxList.push_back( bbox );
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
