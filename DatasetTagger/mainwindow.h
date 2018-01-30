#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QModelIndex>
#include <QHash>
#include <QSettings>

#include "qtrainsetexample.h"
#include "qimagescene.h"

class QGraphicsItem;

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void updateImgList();
    void updateBBoxList();
    void fitImage();
    bool getLabelInfo(int idx, QString& label, QColor& col, bool setIfMatch=true);

    bool initDatasetFromFolder();

protected slots:
    void onImageListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onLabelListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void loadSettings();
    void saveSettings();

    bool saveLabels();
    bool loadLabels();

    void onNewBbox( QGraphicsItem* item, double nx, double ny, double nw, double nh );
    void onRemoveBbox( QGraphicsItem* item );

private slots:
    void on_pushButton_img_folder_clicked();
    void on_pushButton_add_label_clicked();
    void on_pushButton_remove_label_clicked();
    void on_pushButton_base_folder_clicked();
    void on_pushButton_clear_clicked();
    void on_pushButton_fit_image_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_pushButton_save_clicked();

private:
    Ui::MainWindow *ui;

    QString mImgFolder;
    QString mBaseFolder;

    QStandardItemModel* mLabelModel;
    QStandardItemModel*   mImgListModel;
    QStringListModel*   mBBoxListModel;

    QImageScene* mScene;

    int mAutoLabNameCount;

    QHash<QString, QTrainSetExample*> mDataSet;

    QSettings* mIniSettings;
};

#endif // MAINWINDOW_H
