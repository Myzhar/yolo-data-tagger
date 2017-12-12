#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QStringListModel>
#include <QModelIndex>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QHash>

#include "qtrainsetexample.h"

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

private slots:
    void on_pushButton_img_folder_clicked();

    void on_pushButton_add_label_clicked();

    void on_pushButton_remove_label_clicked();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_base_folder_clicked();

private:
    Ui::MainWindow *ui;

    QString mImgFolder;
    QString mBaseFolder;

    QStandardItemModel* mLabelModel;
    QStringListModel* mImgListModel;

    QGraphicsScene* mScene;
    QGraphicsPixmapItem* mImgItem;

    int mAutoLabNameCount;

    QHash<QString, QTrainSetExample*> mDataSet;
};

#endif // MAINWINDOW_H
