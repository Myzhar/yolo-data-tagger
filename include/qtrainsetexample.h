#ifndef QTRAINSETEXAMPLE_H
#define QTRAINSETEXAMPLE_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include "qobjbbox.h"

#include <opencv2/core/core.hpp>

class QTrainSetExample : public QObject
{
    Q_OBJECT

public:
    explicit QTrainSetExample(QString imgFilename, QObject *parent = nullptr);

    void setFullFolderPath( QString fullPath );
    void setRelFolderPath( QString fullPath, QString basePath );
    QString getFullPath();
    QString getRelPath();
    QString getImgName();

    void setTestSet(bool testSet);
    bool isTestSample();

    void addNewBBox(quint64 bboxIdx, int labIdx, double nx, double ny, double nw, double nh );
    void removeBBox( quint64 bboxIdx );

    QStringList getBboxesStrings();
    const QHash<quint64,QObjBBox*>& getBBoxes();

    bool saveYoloFormat();
    bool loadYoloFormat();

    QTrainSetExample* cloneBlur();
    QTrainSetExample* cloneSaltAndPepper();
    QTrainSetExample* cloneFlip( int mode );
    QTrainSetExample* cloneRotateScale( double angleDeg, double scale );

private:
    QObjBBox rotateBBox(QObjBBox* bbox,  cv::Size imgSize, cv::Mat RT);

signals:

public slots:

private:
    QString mImageFilename;
    QString mFullFolderPath;
    QString mRelativeFolderPath;

    bool mTestSet; ///< If true this belongs to TestSet, else TrainingSet

    QHash<quint64,QObjBBox*> mBboxMap;
};

#endif // QTRAINSETEXAMPLE_H
