#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfoList>
#include <QDir>
#include <QColor>
#include <QWheelEvent>
#include <QHashIterator>
#include <QOpenGLWidget>
#include <QTextStream>
#include <QProgressDialog>

#include "qobjbbox.h"

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

    mAutoLabNameCount = 0;

    ui->spinBox->setValue(0);

    // >>>>> Image Rendering
    QOpenGLWidget* oglWidget = new QOpenGLWidget( );

    ui->graphicsView_image->setViewport( oglWidget );

    if( oglWidget->isValid() )
    {
        qDebug() << tr("OpenGL context succesfully initialized");

        ui->graphicsView_image->setViewportUpdateMode(
            QGraphicsView::FullViewportUpdate);
    }
    else
    {
        ui->graphicsView_image->setViewport( new QWidget() );
    }

    mScene = new QImageScene( this );
    ui->graphicsView_image->setScene(mScene);


    connect( mScene, &QImageScene::newBBox,
             this, &MainWindow::onNewBbox );
    connect( mScene, &QImageScene::removedBBox,
             this, &MainWindow::onRemoveBbox );
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

    if( initDatasetFromFolder() )
        updateImgList();
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
    ui->lineEdit_base_folder_path->setToolTip(mBaseFolder);

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
    ui->lineEdit_img_folder_path->setToolTip(mImgFolder);
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

    QString label = tr("[%1] %2")
                    .arg(mLabelModel->item( current.row(), 0 )->text())
                    .arg(mLabelModel->item( current.row(), 1 )->text());

    mScene->setBBoxLabel( label, labelColor );
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

    //updateBBoxList();

    const QHash<quint64,QObjBBox*> bboxes = mDataSet[imageName]->getBBoxes();

    QHashIterator<quint64,QObjBBox*> iter(bboxes);
    while (iter.hasNext())
    {
        iter.next();

        QObjBBox* bbox = iter.value();

        if(!bbox)
            continue;

        int labIdx;
        double nx,ny,nw,nh;
        bbox->getBBox( labIdx,nx,ny,nw,nh );

        QString label = tr("Test");
        QColor color;

        if( getLabelInfo( labIdx, label, color) )
        {
            QString fullLabel = tr("[%1] %2")
                                .arg(labIdx, 3, 10, QChar('0') )
                                .arg(label);

            mDataSet[imageName]->removeBBox( iter.key() );
            mScene->addBBox( fullLabel, color,nx,ny,nw,nh );
        }
    }
}

bool MainWindow::getLabelInfo( int idx, QString& label, QColor& col, bool setIfMatch)
{
    for( int r=0; r<mLabelModel->rowCount(); r++ )
    {
        int currIdx = mLabelModel->item( r, 0 )->text().toInt();

        if( currIdx == idx )
        {
            label = mLabelModel->item( r, 1 )->text();
            col = mLabelModel->item( r, 2 )->background().color();

            if(setIfMatch)
            {
                ui->tableView_labels->setCurrentIndex( mLabelModel->index( r, 1 ) );
            }

            return true;
        }
    }

    return false;
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
    ui->lineEdit_base_folder_path->setToolTip(mBaseFolder);

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
        delete mBBoxListModel;
        mBBoxListModel = NULL;
    }

    QString imageName = mImgListModel->data( ui->listView_images->currentIndex() ).toString();

    if(imageName.isEmpty())
        return;

    QStringList bboxes = mDataSet[imageName]->getBboxesStrings();

    mBBoxListModel = new QStringListModel(bboxes);
    ui->listView_bounding_boxes->setModel( mBBoxListModel );
}

bool MainWindow::initDatasetFromFolder()
{
    QDir imgDir( mImgFolder );

    if( !imgDir.exists() )
    {
        return false;
    }

    QStringList fileTypes;
    fileTypes << "*.jpg" << "*.jpeg" << "*.png";
    imgDir.setNameFilters(fileTypes);

    QStringList imgList = imgDir.entryList( fileTypes, QDir::Files|QDir::NoSymLinks, QDir::Name|QDir::Name );

    if(imgList.size()==0)
    {
        return false;
    }

    // >>>>> Dataset initialization
    mDataSet.clear();

    foreach(QString imgFile, imgList)
    {
        QTrainSetExample* ts = new QTrainSetExample(imgFile);

        ts->setRelFolderPath( mImgFolder, mBaseFolder );

        mDataSet[imgFile] = ts;

        ts->loadYoloFormat();
    }

    loadLabels();
    // <<<<< Dataset initialization

    return true;
}

void MainWindow::updateImgList()
{
    int prevRow = ui->listView_images->currentIndex().row();

    if( mDataSet.size()==0 )
        return;

    if( mImgListModel )
    {
        QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
        disconnect(imgSelModel,&QItemSelectionModel::currentChanged,
                   this, &MainWindow::onImageListCurrentChanged );

        delete mImgListModel;
        mImgListModel = NULL;
    }

    ui->listView_images->setViewMode( QListView::ListMode );

    mImgListModel = new QStandardItemModel();

    QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
    while (iter.hasNext())
    {
        iter.next();

        QTrainSetExample* sample = iter.value();

        mImgListModel->appendRow(new QStandardItem(iter.key()));
        mImgListModel->setData(mImgListModel->index(mImgListModel->rowCount()-1, 0),
                               sample->isTestSample()?QBrush(Qt::yellow):QBrush(Qt::lightGray),
                               Qt::BackgroundRole);
    }

    mImgListModel->sort( 0 );
    ui->listView_images->setModel( mImgListModel );

    QItemSelectionModel *imgSelModel = ui->listView_images->selectionModel();
    connect(imgSelModel,&QItemSelectionModel::currentChanged,
            this, &MainWindow::onImageListCurrentChanged );

    if( prevRow != -1 )
    {
        ui->listView_images->setCurrentIndex( mLabelModel->index( prevRow, 0 ));
    }
}

void MainWindow::on_pushButton_img_folder_clicked()
{
    mImgFolder = QFileDialog::getExistingDirectory( this, tr("Image folder"), mBaseFolder );

    if( mImgFolder.isEmpty() )
    {
        return;
    }

    saveSettings();

    if( initDatasetFromFolder() )
        updateImgList();

    ui->lineEdit_img_folder_path->setText( mImgFolder );
    ui->lineEdit_img_folder_path->setToolTip(mImgFolder);
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
        label = tr("Label_%1").arg(mAutoLabNameCount,3,10,QChar('0'));
    }
    labelItem->setText( label );
    labelItem->setEditable( true );

    QStandardItem* colorItem = new QStandardItem();

    quint8 r = (mAutoLabNameCount+9)*30;
    quint8 g = (mAutoLabNameCount+6)*60;
    quint8 b = (mAutoLabNameCount+3)*90;

    colorItem->setText(  tr("%1,%2,%3").arg(r,3,10,QChar('0')).arg(g,3,10,QChar('0')).arg(b,3,10,QChar('0')) );
    colorItem->setEditable( false );
    colorItem->setBackground( QBrush( QColor(r,g,b)) );
    colorItem->setForeground( QBrush( QColor(255-r,255-g,255-b)) );
    colorItem->setSelectable(false);

    QList<QStandardItem*> row;
    row << idxItem << labelItem << colorItem;
    mLabelModel->appendRow( row );

    ui->tableView_labels->setCurrentIndex( mLabelModel->index( mLabelModel->rowCount()-1, 1 ) );

    mAutoLabNameCount++;
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
    updateBBoxList();
}

void MainWindow::on_pushButton_fit_image_clicked()
{
    fitImage();
}

void MainWindow::onNewBbox(QGraphicsItem *item, double nx, double ny, double nw, double nh)
{
    QString imageName = mImgListModel->data( ui->listView_images->currentIndex() ).toString();

    if(imageName.isEmpty())
        return;

    QModelIndex labelIdx = ui->tableView_labels->currentIndex();
    QString labIdxStr = mLabelModel->item( labelIdx.row(), 0 )->text();

    if(labIdxStr.isEmpty())
        return;

    mDataSet[imageName]->addNewBBox( (quint64)item, labIdxStr.toInt(), nx, ny, nw, nh );

    updateBBoxList();
}

void MainWindow::onRemoveBbox(QGraphicsItem *item)
{
    QString imageName = mImgListModel->data( ui->listView_images->currentIndex() ).toString();

    if(imageName.isEmpty())
        return;

    mDataSet[imageName]->removeBBox( (quint64)item );

    updateBBoxList();
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    double perc = static_cast<double>(arg1)/100.;

    double count = mDataSet.count();

    int valCount = static_cast<int>(count*perc-0.5);

    int step = static_cast<int>(count/valCount+0.5);

    for( int r=0; r<mImgListModel->rowCount(); r++ )
    {
        QString imageName = mImgListModel->data( mImgListModel->index( r,0 ) ).toString();

        if( (r%step)==0 )
        {
            mDataSet[imageName]->setTestSet(true);
        }
        else
        {
            mDataSet[imageName]->setTestSet(false);
        }
    }

    updateImgList();
}

void MainWindow::on_pushButton_save_clicked()
{
    QProgressDialog prgDlg( this );
    prgDlg.setWindowTitle( tr("Saving...") );
    prgDlg.setCancelButton( 0 );

    prgDlg.setWindowModality(Qt::WindowModal);

    int count = 0;

    if( ui->groupBox_augmentation->isChecked() )
    {
        if( ui->checkBox_aug_H_flip->isChecked() )
        {
            count = 0;

            // >>>>> Add Flipped images and update list
            prgDlg.setLabelText( tr("Adding Horiz. flipped samples...") );
            prgDlg.setMinimum( 0 );
            prgDlg.setMaximum( mDataSet.size() );

            QHash<QString, QTrainSetExample*> newDS = mDataSet;

            QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
            while (iter.hasNext())
            {
                iter.next();

                QTrainSetExample* sample = iter.value();

                QTrainSetExample* spSample = sample->cloneFlip(1);

                if(spSample)
                {
                    spSample->setRelFolderPath( mImgFolder, mBaseFolder );
                    newDS[spSample->getImgName()] = spSample;
                }

                prgDlg.setValue( ++count );
                QApplication::processEvents( QEventLoop::AllEvents, 5 );
            }

            mDataSet = newDS;
            // <<<<< Add Flipped images and update list
        }

        if( ui->checkBox_aug_V_flip->isChecked() )
        {
            count = 0;

            // >>>>> Add Flipped images and update list
            prgDlg.setLabelText( tr("Adding Vert. flipped samples...") );
            prgDlg.setMinimum( 0 );
            prgDlg.setMaximum( mDataSet.size() );

            QHash<QString, QTrainSetExample*> newDS = mDataSet;

            QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
            while (iter.hasNext())
            {
                iter.next();

                QTrainSetExample* sample = iter.value();

                QTrainSetExample* spSample = sample->cloneFlip(0);

                if(spSample)
                {
                    spSample->setRelFolderPath( mImgFolder, mBaseFolder );
                    newDS[spSample->getImgName()] = spSample;
                }

                prgDlg.setValue( ++count );
                QApplication::processEvents( QEventLoop::AllEvents, 5 );
            }

            mDataSet = newDS;
            // <<<<< Add Flipped images and update list
        }

        if( ui->checkBox_aug_salt_pepper->isChecked() )
        {
            count = 0;

            // >>>>> Add Salt&Pepper images and update list
            prgDlg.setLabelText( tr("Adding Salt&Pepper samples...") );
            prgDlg.setMinimum( 0 );
            prgDlg.setMaximum( mDataSet.size() );

            QHash<QString, QTrainSetExample*> newDS = mDataSet;

            QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
            while (iter.hasNext())
            {
                iter.next();

                QTrainSetExample* sample = iter.value();

                QTrainSetExample* spSample = sample->cloneSaltAndPepper();

                if(spSample)
                {
                    spSample->setRelFolderPath( mImgFolder, mBaseFolder );
                    newDS[spSample->getImgName()] = spSample;
                }

                prgDlg.setValue( ++count );
                QApplication::processEvents( QEventLoop::AllEvents, 5 );
            }

            mDataSet = newDS;
            // <<<<< Add Salt&Pepper images and update list
        }

        if( ui->checkBox_aug_blur->isChecked() )
        {
            count = 0;

            // >>>>> Add blurred images and update list

            prgDlg.setLabelText( tr("Adding Blurred samples...") );
            prgDlg.setMinimum( 0 );
            prgDlg.setMaximum( mDataSet.size() );


            QHash<QString, QTrainSetExample*> newDS = mDataSet;

            QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
            while (iter.hasNext())
            {
                iter.next();

                QTrainSetExample* sample = iter.value();

                QTrainSetExample* blurSample = sample->cloneBlur();

                if(blurSample)
                {
                    blurSample->setRelFolderPath( mImgFolder, mBaseFolder );
                    newDS[blurSample->getImgName()] = blurSample;
                }

                prgDlg.setValue( ++count );
                QApplication::processEvents( QEventLoop::AllEvents, 5 );
            }

            mDataSet = newDS;
            // <<<<< Add blurred images and update list
        }
    }

    QString relPath;

    // >>>>> Train & Test files list on files
    if( !mImgFolder.endsWith("/") )
        mImgFolder += "/";

    QString trainFilename = tr("%1train.txt").arg(mImgFolder);
    QString testFilename = tr("%1test.txt").arg(mImgFolder);

    QFile trainFile(trainFilename);
    QFile testFile(testFilename);

    if( !trainFile.open( QFile::WriteOnly|QFile::Text ) )
    {
        // TODO ADD ERROR
        return;
    }

    if( !testFile.open( QFile::WriteOnly|QFile::Text ) )
    {
        // TODO ADD ERROR
        return;
    }

    QTextStream trainStream( &trainFile );
    QTextStream testStream( &testFile );

    prgDlg.setLabelText( tr("Saving Samples data...") );
    prgDlg.setMaximum(mDataSet.size());
    count = 0;

    QHashIterator<QString, QTrainSetExample*> iter(mDataSet);
    while (iter.hasNext())
    {
        iter.next();

        QString imageName = iter.value()->getImgName();

        mDataSet[imageName]->saveYoloFormat();

        if(!iter.hasPrevious())
        {
            relPath = mDataSet[imageName]->getRelPath();
            if( !relPath.endsWith("/"))
                relPath += "/";
        }

        QString dataLine = mDataSet[imageName]->getRelPath();
        if( !dataLine.endsWith("/"))
            dataLine += "/";

        dataLine += imageName;

        if( mDataSet[imageName]->isTestSample() )
        {
            testStream << dataLine << endl;
        }
        else
        {
            trainStream << dataLine << endl;
        }

        prgDlg.setValue( ++count );
    }
    // <<<<< Train & Test files list on files

    saveLabels();

    // >>>>> Training Set Data
    QString dataFilename = tr("%1trainingSet.data").arg(mImgFolder);

    QFile dataFile(dataFilename);

    if( !dataFile.open( QFile::WriteOnly|QFile::Text ) )
    {
        // TODO ADD ERROR
        return;
    }

    QTextStream dataStream( &dataFile );

    int nClasses = mLabelModel->rowCount();

    dataStream << tr("classes = %1").arg(nClasses) << endl;
    dataStream << tr("train   = %1train.txt").arg(relPath) << endl;
    dataStream << tr("valid   = %1test.txt").arg(relPath) << endl;
    dataStream << tr("names   = %1label.names").arg(relPath) << endl;
    dataStream << tr("backup  = backup") << endl;
    // <<<<< Training Set Data
}

bool MainWindow::saveLabels()
{
    // >>>>> Label names
    if( !mImgFolder.endsWith("/") )
        mImgFolder += "/";

    QString labelFilename = tr("%1label.names").arg(mImgFolder);

    QFile labelFile(labelFilename);

    if( !labelFile.open( QFile::WriteOnly|QFile::Text ) )
    {
        // TODO ADD ERROR
        return false;
    }

    QTextStream labelsStream( &labelFile );

    int nClasses = mLabelModel->rowCount();

    for( int r=0; r<nClasses; r++ )
    {
        QString labelName = mLabelModel->item( r, 1 )->text();

        labelsStream << labelName << endl;
    }
    // <<<<< Label names

    return true;
}

bool MainWindow::loadLabels()
{
    // >>>>> Label names
    if( !mImgFolder.endsWith("/") )
        mImgFolder += "/";

    QString labelFilename = tr("%1label.names").arg(mImgFolder);

    QFile labelFile(labelFilename);

    if( !labelFile.open( QFile::ReadOnly|QFile::Text ) )
    {
        // TODO ADD ERROR
        return false;
    }

    mAutoLabNameCount=0;
    mLabelModel->clear();

    QTextStream labelsStream( &labelFile );

    while( !labelsStream.atEnd() )
    {
        QString label = labelsStream.readLine();

        if( !label.isEmpty() )
        {
            ui->lineEdit_label->setText(label);
            on_pushButton_add_label_clicked();
        }
    }
    // <<<<< Label names

    return true;
}
