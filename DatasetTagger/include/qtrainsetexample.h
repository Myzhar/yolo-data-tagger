#ifndef QTRAINSETEXAMPLE_H
#define QTRAINSETEXAMPLE_H

#include <QObject>
#include <QHash>
#include <QStringList>
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
    bool isTestSample();

    void addBBox(quint64 bboxIdx, int labIdx, double nx, double ny, double nw, double nh );
    void removeBBox( quint64 bboxIdx );

    QStringList getBboxesStrings();
    const QHash<quint64,QObjBBox*>& getBBoxes();

    bool saveYoloFormat();
    bool loadYoloFormat();

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
