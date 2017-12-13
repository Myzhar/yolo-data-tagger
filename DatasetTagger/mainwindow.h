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

protected slots:
    void onImageListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);
    void onLabelListCurrentChanged(const QModelIndex &current, const QModelIndex &previous);

    void loadSettings();
    void saveSettings();

    void updateImgList();

private slots:
    void on_pushButton_img_folder_clicked();
    void on_pushButton_add_label_clicked();
    void on_pushButton_remove_label_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_pushButton_base_folder_clicked();

    void on_pushButton_clear_clicked();

private:
    Ui::MainWindow *ui;

    QString mImgFolder;
    QString mBaseFolder;

    QStandardItemModel* mLabelModel;
    QStringListModel* mImgListModel;

    QImageScene* mScene;

    int mAutoLabNameCount;

    QHash<QString, QTrainSetExample*> mDataSet;

    QSettings* mIniSettings;
};

#endif // MAINWINDOW_H
