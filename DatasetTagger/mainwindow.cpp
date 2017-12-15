#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QFileInfoList>
#include <QDir>
#include <QColor>
#include <QWheelEvent>
#include <QHashIterator>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mLabelModel(NULL),
    mImgListModel(NULL),
    mBBoxListModel(NULL),
    mScene(NULL),
    mIniSettings(NULL)
{
    ui->setupUi(this);

    loadSettings();
    updateImgList();

    mAutoLabNameCount = 0;

    // >>>>> Image Rendering
    mScene = new QImageScene( this );
    ui->graphicsView_image->setScene(mScene);
    // <<<<< Image Rendering

    // >>>>> Label table
    mLabelModel = new QStandardItemModel( 0, 3 );
    mLabelModel->setHorizontalHeaderItem(0, new QStandardItem(QString("Idx")));
    mLabelModel->setHorizontalHeaderItem(1, new QStandardItem(QString("Name")));
    mLabelModel->setHorizontalHeaderItem(2, new QStandardItem(QString("Color")));

    ui->tableView_labels->setModel(mLabelModel);

    QItemSelectionModel* labelSelModel = ui->tableView_labels->selectionModel();
    connect( labelSelModel, &QItemSelectionModel::currentChanged,
             this, &MainWindow::onLabelListCurrentChanged );
    // <<<<< Label table
}

MainWindow::~MainWindow()
{
    delete ui;

    if( mIniSettings )
        delete mIniSettings;
}

void MainWindow::loadSettings()
{
    mBaseFolder = "";
    mImgFolder = "";

    if( !mIniSettings )
    {
        QString iniFile = QApplication::applicationDirPath();
        if( !iniFile.endsWith("/") )
        {
            iniFile += "/";
        }
        iniFile += QApplication::applicationName();
        iniFile+= ".ini";

        mIniSettings = new QSettings( iniFile, QSettings::IniFormat );
    }

    mBaseFolder = mIniSettings->value( "BASE_FOLDER", QString() ).toString();
    ui->lineEdit_base_folder_path->setText(mBaseFolder);

    if( !mBaseFolder.isEmpty() )
    {
        ui->pushButton_img_folder->setEnabled(true);
        mImgFolder = mIniSettings->value( "IMAGE_FOLDER", QString() ).toString();
    }
    else
    {
        ui->pushButton_img_folder->setEnabled(false);
    }

    ui->lineEdit_img_folder_path->setText(mImgFolder);
}

void MainWindow::saveSettings()
{
    if( !mIniSettings )
    {
        QString iniFile = QApplication::applicationDirPath();
        if( !iniFile.endsWith("/") )
        {
            iniFile += "/";
        }
        iniFile += QApplication::applicationName();
        iniFile+= ".ini";

        mIniSettings = new QSettings( iniFile, QSettings::IniFormat );
    }

    mIniSettings->setValue( "BASE_FOLDER", mBaseFolder );
    mIniSettings->setValue( "IMAGE_FOLDER", mImgFolder );
}


void MainWindow::onLabelListCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    QStandardItem* item = mLabelModel->item( current.row(), 2 );

    if( item==NULL )
    {
        mScene->enableDrawing(false);
        return;
    }

    QColor labelColor = item->background().color();

    mScene->setBBoxLabel( mLabelModel->item( current.row(), 1 )->text(), labelColor );
}

void MainWindow::onImageListCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    QString imageName = mImgListModel->data( current ).toString();

    QString currImagePath = mDataSet[imageName]->getFullPath();
    if( !currImagePath.endsWith( "/") )
        currImagePath += "/";
    currImagePath += imageName;

    QImage image(currImagePath);

    if( image.isNull() )
        return;

    mScene->removeAllBBoxes();
    mScene->setImage( image );

    fitImage();
}

void MainWindow::on_pushButton_base_folder_clicked()
{
    mBaseFolder = QFileDialog::getExistingDirectory( this, tr("Base folder for relative paths"),
                  QDir::homePath() );

    if( mBaseFolder.isEmpty() )
    {
        ui->pushButton_img_folder->setEnabled(false);
        return;
    }

    saveSettings();

    ui->lineEdit_base_folder_path->setText(mBaseFolder);

    ui->pushButton_img_folder->setEnabled(true);
}

void MainWindow::fitImage()
{
    QSizeF imgSize = mScene->getImageSize();

    ui->graphicsView_image->setSceneRect( 0,0, imgSize.width(), imgSize.height() );
    ui->graphicsView_image->fitInView(QRectF(0,0, imgSize.width(), imgSize.height()),
                                      Qt::KeepAspectRatio );
    ui->graphicsView_image->update();
}

void MainWindow::updateBBoxList()
{
    if( mBBoxListModel )
    {
        QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();

        delete mImgListModel;
        mImgListModel = NULL;
    }
}

void MainWindow::updateImgList()
{
    ui->comboBox_ts_perc->setCurrentIndex(0);

    if( mImgListModel )
    {
        QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
        disconnect(imgSelModel,&QItemSelectionModel::currentChanged,
                   this, &MainWindow::onImageListCurrentChanged );

        delete mImgListModel;
        mImgListModel = NULL;
    }

    ui->listView_images->setViewMode( QListView::ListMode );

    QDir imgDir( mImgFolder );

    if( !imgDir.exists() )
    {
        return;
    }

    QStringList fileTypes;
    fileTypes << "*.jpg" << "*.jpeg" << "*.png";
    imgDir.setNameFilters(fileTypes);

    QStringList imgList = imgDir.entryList( fileTypes, QDir::Files|QDir::NoSymLinks, QDir::Name|QDir::Name );

    mImgListModel = new QStringListModel(imgList);
    ui->listView_images->setModel( mImgListModel );

    QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
    connect(imgSelModel,&QItemSelectionModel::currentChanged,
            this, &MainWindow::onImageListCurrentChanged );

    ui->listView_images->setCurrentIndex( QModelIndex());

    // >>>>> Dataset initialization
    mDataSet.clear();

    foreach(QString imgFile, imgList)
    {
        QTrainSetExample* ts = new QTrainSetExample(imgFile);

        ts->setRelFolderPath( mImgFolder, mBaseFolder );

        mDataSet[imgFile] = ts;
    }
    // <<<<< Dataset initialization
}

void MainWindow::on_pushButton_img_folder_clicked()
{
    mImgFolder = QFileDialog::getExistingDirectory( this, tr("Image folder"), mBaseFolder );

    if( mImgFolder.isEmpty() )
    {
        return;
    }

    saveSettings();
    updateImgList();

    ui->lineEdit_img_folder_path->setText( mImgFolder );
}


void MainWindow::on_pushButton_add_label_clicked()
{
    int idx = mLabelModel->rowCount();

    QStandardItem* idxItem = new QStandardItem();
    idxItem->setText( tr("%1").arg(idx,3,10,QChar('0')) );
    idxItem->setEditable( false );
    idxItem->setTextAlignment( Qt::AlignHCenter|Qt::AlignVCenter );

    QStandardItem* labelItem = new QStandardItem();
    QString label = ui->lineEdit_label->text();
    if( label.isEmpty() )
    {
        label = tr("Label_%1").arg(mAutoLabNameCount++,3,10,QChar('0'));
    }
    labelItem->setText( label );
    labelItem->setEditable( true );

    QStandardItem* colorItem = new QStandardItem();

    quint8 r = (idx+9)*30;
    quint8 g = (idx+6)*60;
    quint8 b = (idx+3)*90;

    colorItem->setText(  tr("%1,%2,%3").arg(r,3,10,QChar('0')).arg(g,3,10,QChar('0')).arg(b,3,10,QChar('0')) );
    colorItem->setEditable( false );
    colorItem->setBackground( QBrush( QColor(r,g,b)) );
    colorItem->setForeground( QBrush( QColor(255-r,255-g,255-b)) );
    colorItem->setSelectable(false);

    QList<QStandardItem*> row;
    row << idxItem << labelItem << colorItem;
    mLabelModel->appendRow( row );

    ui->tableView_labels->setCurrentIndex( mLabelModel->index( mLabelModel->rowCount()-1, 1 ) );
}

void MainWindow::on_pushButton_remove_label_clicked()
{
    int idx = ui->tableView_labels->currentIndex().row();

    mLabelModel->removeRow( idx  );

    for( int i=idx; i<mLabelModel->rowCount(); i++ )
    {
        QStandardItem* idxItem = mLabelModel->item( i, 0 );

        if(!idxItem)
            continue;

        idxItem->setText( tr("%1").arg(i,3,10,QChar('0')) );
    }
}

void MainWindow::on_pushButton_clear_clicked()
{
    mScene->removeAllBBoxes();
}

void MainWindow::on_pushButton_fit_image_clicked(bool checked)
{
    fitImage();
}

void MainWindow::on_comboBox_ts_perc_currentIndexChanged(int index)
{
    double perc = 0.0;

    switch(index)
    {
    case 1:
        perc = 0.01;
        break;

    case 2:
        perc=0.05;
        break;

    case 3:
        perc=0.1;
        break;

    case 4:
        perc=0.2;
        break;

    case 5:
        perc=0.5;
        break;

    case 6:
        perc=1.0;
        break;

    default:
        perc = 0.0;
    }

    double count = mDataSet.count();

    int valCount = static_cast<int>(qRound(count*perc));

    int step = static_cast<int>(qRound(count/valCount));

    int idx=1;

    QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
    while (iter.hasNext())
    {
        iter.next();

        if( (idx%step)==0 )
        {
            iter.value()->setTestSet(true);
        }
        else
        {
            iter.value()->setTestSet(false);
        }

        idx++;
    }

}
