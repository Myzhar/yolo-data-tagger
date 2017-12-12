#ifndef QTRAINSETEXAMPLE_H
#define QTRAINSETEXAMPLE_H

#include <QObject>
#include "qobjbbox.h"

class QTrainSetExample : public QObject
{
    Q_OBJECT
public:
    explicit QTrainSetExample(QString imgFilename, QObject *parent = nullptr);

    void setFullFolderPath( QString fullPath );
    void setRelFolderPath( QString fullPath, QString basePath );
    QString getFullPath();
    QString getRelPath();

    void setTestSet(bool testSet);

    void addBBox( int labIdx, double nx, double ny, double nw, double nh );
signals:

public slots:

private:
    QString mImageFilename;
    QString mFullFolderPath;
    QString mRelativeFolderPath;

    bool mTestSet; ///< If true this belogs to TestSet, else TrainingSet

    QList<QObjBBox> mBboxList;
};

#endif // QTRAINSETEXAMPLE_H
