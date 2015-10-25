#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFtp>
#include <QDebug>
#include <QFile>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QTreeWidgetItem>
#include <QXmlStreamReader>
#include <QAbstractItemView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->initDisplay();
    this->showLocalFile();
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showLocalTreeViewMenu(QPoint)));
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::initDisplay()
{
    ui->cdToParentButton->setEnabled(false);
    ui->downloadButton->setEnabled(false);
    ui->uploadButton->setEnabled(false);
    ui->progressBar->setValue(0);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    //fileLish 可使用右键菜单
    ui->fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    this->m_server_menu = new QMenu(this);

    QAction *server_rm = new QAction(QObject::tr("删除"),this);
    this->m_server_menu->addAction(server_rm);
}
void MainWindow::ftpCommandStarted(int)
{
    int id = ftp->currentCommand();
    switch (id)
    {
    case QFtp::ConnectToHost :
        ui->label->setText(tr("正在连接到服务器…"));
        break;
    case QFtp::Login :
        ui->label->setText(tr("正在登录…"));
        break;
    case QFtp::Get :
        ui->label->setText(tr("正在下载…"));
        break;
    case QFtp::Close :
        ui->label->setText(tr("正在关闭连接…"));
    }
}

void MainWindow::ftpCommandFinished(int, bool error)
{
    if(ftp->currentCommand() == QFtp::ConnectToHost) {
        if (error){
            ui->label->setText(tr("连接服务器出现错误：%1").arg(ftp->errorString()));
        }
        else{
            ui->connectButton->setDisabled(true);
            ui->connectButton->setText("已连接");
            ui->label->setText(tr("连接到服务器成功"));
        }
    } else if (ftp->currentCommand() == QFtp::Login) {
        if (error){
            ui->label->setText(tr("登录出现错误：%1").arg(ftp->errorString()));
        }
        else {
            ui->downloadButton->setEnabled(true);
            ui->uploadButton->setEnabled(true);
            ui->label->setText(tr("登录成功"));
            ftp->list();
        }
    } else if (ftp->currentCommand() == QFtp::Get) {
        if(error) ui->label->setText(tr("下载出现错误：%1").arg(ftp->errorString()));
        else {
            file->close();
            ui->label->setText(tr("下载完成"));
            currentIndex ++;
            if(currentIndex < indexCount){
                this->downloadFtpFile(currentIndex);
            }
            else{
                 currentIndex = 0;
                 dirModel->refresh();
            }
        }
        ui->downloadButton->setEnabled(true);
    } else if (ftp->currentCommand() == QFtp::List){
        if (isDirectory.isEmpty())
        {
            ui->fileList->addTopLevelItem(
                        new QTreeWidgetItem(QStringList()<< tr("<empty>")));
            ui->fileList->setEnabled(false);
            ui->label->setText(tr("该目录为空"));
        }
    }else if (ftp->currentCommand() == QFtp::Put) {
        if(error) ui->label->setText(tr("上传出现错误：检查文件是否重名！").arg(ftp->errorString()));
        else {
            ui->label->setText(tr("上传完成"));
            file->close();
            currentIndex ++;
            if(currentIndex < indexCount){
                this->uploadLocalFile(currentIndex);
            }
            else{
                currentIndex = 0;
                //刷新显示列表
                isDirectory.clear();
                ui->fileList->clear();
                ftp->list();
            }
        }
    }else if (ftp->currentCommand() == QFtp::Mkdir){
        ui->label->setText(tr("新建文件夹完成"));
        //刷新显示列表
        isDirectory.clear();
        ui->fileList->clear();
        ftp->list();
    }else if (ftp->currentCommand() == QFtp::Remove){
        currentIndex++;
        if(currentIndex >= indexCount){
            ui->label->setText(tr("删除完成！"));
            isDirectory.clear();
            ui->fileList->clear();
            ftp->list();
        }
    }else if(ftp->currentCommand() == QFtp::Rmdir){
        currentIndex++;
        if(currentIndex >= indexCount){
            ui->label->setText(tr("删除完成！"));
            isDirectory.clear();
            ui->fileList->clear();
            ftp->list();
        }
    }
    else if (ftp->currentCommand() == QFtp::Close) {
        ui->label->setText(tr("已经关闭连接"));
    }
}

// 连接按钮
void MainWindow::on_connectButton_clicked()
{
    ui->fileList->clear();
    currentPath.clear();
    isDirectory.clear();

    ftp = new QFtp(this);
    connect(ftp, SIGNAL(commandStarted(int)), this, SLOT(ftpCommandStarted(int)));
    connect(ftp, SIGNAL(commandFinished(int, bool)),
            this, SLOT(ftpCommandFinished(int, bool)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(addToList(QUrlInfo)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64, qint64)),
            this, SLOT(updateDataTransferProgress(qint64, qint64)));

    connect(ui->fileList, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showFtpTreeViewMenu(QPoint)));
    //双击进入目录
    connect(ui->fileList, SIGNAL(itemActivated(QTreeWidgetItem*, int)),
            this, SLOT(processItem(QTreeWidgetItem*, int)));

    QString ftpServer = ui->ftpServerLineEdit->text();
    QString userName = "anonymous";
    QString passWord = "admin123";
    ftp->connectToHost(ftpServer,SERVERPORT);
    ftp->login(userName, passWord);
}

void MainWindow::addToList(const QUrlInfo &urlInfo)
{
    //
    if(_FromSpecialEncoding(urlInfo.name()).startsWith("."))
        return;
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, _FromSpecialEncoding(urlInfo.name()));
    double  dFileSize = ((int)(urlInfo.size()/1024.0*100))/100.0;
    QString fileSize = QString::number(dFileSize,'g',10)+"KB";

    item->setText(1, fileSize);
    item->setText(2, urlInfo.lastModified().toString("MMM dd yyyy"));
    item->setText(3, urlInfo.owner());
    item->setText(4, urlInfo.group());
    QPixmap pixmap(urlInfo.isDir() ? "./dir.png" : "./file.png");
    item->setIcon(0, pixmap);
    isDirectory[_FromSpecialEncoding(urlInfo.name())] = urlInfo.isDir();
    ui->fileList->addTopLevelItem(item);
    if (!ui->fileList->currentItem()) {
        ui->fileList->setCurrentItem(ui->fileList->topLevelItem(0));
        ui->fileList->setEnabled(true);
    }
}

void MainWindow::processItem(QTreeWidgetItem *item, int)
{
    if(item->isDisabled())
        return;
    QString name = item->text(0);
    // 如果这个文件是个目录，则打开
    if (isDirectory.value(name)) {
        ui->fileList->clear();
        isDirectory.clear();
        currentPath += "/";
        currentPath += name;
        ftp->cd(_ToSpecialEncoding(name));
        ftp->list();    //重新显示文件列表
        ui->cdToParentButton->setEnabled(true);
    }
}

// 返回上级目录按钮
void MainWindow::on_cdToParentButton_clicked()
{
    qDebug()<<"re:currentPath:"<<currentPath;
    ui->fileList->clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    qDebug()<<"now:currentPath:"<<currentPath;
    if (currentPath.isEmpty()) {
        ui->cdToParentButton->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(_ToSpecialEncoding(currentPath));
    }
    ftp->list();
}

// 下载按钮
void MainWindow::on_downloadButton_clicked()
{
    //防止目录为空时，点击下载按钮出错
    if(isDirectory.isEmpty())
        return;
    //测试下载路径
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        ui->label->setText("请选择正确的下载路径！");
        return;
    }
    //获取下载文件信息

    indexCount = ui->fileList->selectionModel()->selectedRows().count();
    if (indexCount <= 0) {
        return;
    }
    ui->downloadButton->setEnabled(false);
    currentIndex = 0;
    this->downloadFtpFile(currentIndex);
}
void MainWindow::downloadFtpFile(int rowIndex)
{
    QModelIndex index = ui->treeView->currentIndex();
    QModelIndexList indexList = ui->fileList->selectionModel()->selectedRows();
    QFileInfo fileInfo = dirModel->fileInfo(index);
    QString fileName;
    if(fileInfo.isDir())
        fileName = fileInfo.absoluteFilePath() + indexList.at(rowIndex).data().toString();
    else
        fileName = fileInfo.absolutePath() + "/" + indexList.at(rowIndex).data().toString();
    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        delete file;
        return;
    }
    ftp->get(_ToSpecialEncoding(indexList.at(rowIndex).data().toString()), file);
}

void MainWindow::updateDataTransferProgress(qint64 readBytes,qint64 totalBytes)
{
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(readBytes);
}

//本地目录操作
void MainWindow::showLocalFile()
{
        dirModel = new QDirModel;

        dirModel->setReadOnly(false);
        dirModel->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);

        //QTreeView treeView = new QTreeView;
        ui->treeView->setModel(dirModel);
       // ui->treeView->setRootIndex(dirModel->index("c:\\"));
        ui->treeView->header()->setStretchLastSection(true);
        ui->treeView->header()->setSortIndicator(0, Qt::AscendingOrder);
        ui->treeView->header()->setSortIndicatorShown(true);
        ui->treeView->header()->setClickable(true);

        QModelIndex index = dirModel->index(QDir::currentPath());
        ui->treeView->expand(index);
        ui->treeView->scrollTo(index);
        ui->treeView->resizeColumnToContents(0);

        QPushButton *createBtn = new QPushButton(tr("Create Directory..."));
        QPushButton *delBtn = new QPushButton(tr("Remove"));

        connect(createBtn, SIGNAL(clicked()), this, SLOT(mkdir()));
        connect(delBtn, SIGNAL(clicked()), this, SLOT(rm()));
}

void MainWindow::mkdir()
{
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        return;
    }
    QString dirName = QInputDialog::getText(this,
                                            tr("新建文件夹"),
                                            tr("文件夹名称"));
    if (!dirName.isEmpty()) {
        if (!dirModel->mkdir(index, dirName).isValid()) {
            QMessageBox::information(this,
                                     tr("新建文件夹"),
                                     tr("创建文件夹失败"));
        }
    }
}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{

}
//文件上传
void MainWindow::on_uploadButton_clicked()
{  
    indexCount = ui->treeView->selectionModel()->selectedRows().count();
    if (indexCount <= 0) {
        return;
    }
    currentIndex = 0;
    this->uploadLocalFile(currentIndex);
}

void MainWindow::uploadLocalFile(int rowIndex)
{
    QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
    QFileInfo fileInfo = dirModel->fileInfo(indexList.at(rowIndex));
    QString fileName = fileInfo.absoluteFilePath();
    file = new QFile(fileName);
    if (!file->open(QIODevice::ReadOnly)) {
        delete file;
        return;
    }
   ftp->put(file,_ToSpecialEncoding(fileInfo.fileName()));
}




//有FTP端编码转本地编码
QString MainWindow::_FromSpecialEncoding(const QString &InputStr)
{
#ifdef Q_OS_WIN
    return  QString::fromLocal8Bit(InputStr.toLatin1());
#else
    QTextCodec *codec = QTextCodec::codecForName("gbk");
    if (codec)
    {
        return codec->toUnicode(InputStr.toLatin1());
    }
    else
    {
        return QString("");
    }
#endif
}
//put get等上传文件时，转换为FTP端编码
QString MainWindow::_ToSpecialEncoding(const QString &InputStr)
{
#ifdef Q_OS_WIN
    return QString::fromLatin1(InputStr.toLocal8Bit());
#else
    QTextCodec *codec= QTextCodec::codecForName("gbk");
    if (codec)
    {
        return QString::fromLatin1(codec->fromUnicode(InputStr));
    }
    else
    {
        return QString("");
    }
#endif
}
//fileList 右键菜单
void MainWindow::showFtpTreeViewMenu(const QPoint &pos)
{

    QMenu* menu=new QMenu;
    menu->addAction(QString(tr("新建文件夹")),this,SLOT(slotMkdir()));
    menu->addAction(QString(tr("刷新")),this,SLOT(slotRefreshFtpList()));
    menu->addAction(QString(tr("删除")),this,SLOT(slotDeleteFile()));
    menu->exec(QCursor::pos());
}

void MainWindow::slotRefreshFtpList()
{
    isDirectory.clear();
    ui->fileList->clear();
    ftp->list();
}

void MainWindow::slotMkdir()
{
    QString dirName = QInputDialog::getText(this,
                                            tr("新建文件夹"),
                                            tr("文件夹名称"));
    if (!dirName.isEmpty()) {
        ftp->mkdir(_ToSpecialEncoding(dirName));
    }
}

void MainWindow::slotDeleteFile()
{
    QModelIndexList indexList = ui->fileList->selectionModel()->selectedRows();
    QString fileName;
    currentIndex = 0;
    indexCount = indexList.count();
    ui->label->setText("正在删除中......请稍等！");
    for(int i = 0;i < indexCount;i++)
    {
        fileName = indexList.at(i).data().toString();
        if(isDirectory.value(fileName))
            ftp->rmdir(_ToSpecialEncoding(fileName));
        else
            ftp->remove(_ToSpecialEncoding(fileName));
    }
}

//本地文件 右键操作
void MainWindow::showLocalTreeViewMenu(const QPoint &pos)
{
    QMenu* menu=new QMenu;
    menu->addAction(QString(tr("新建文件夹")),this,SLOT(mkdir()));
    menu->addAction(QString(tr("删除")),this,SLOT(rm()));
    menu->addAction(QString(tr("刷新")),this,SLOT(localDirRefresh()));
    menu->exec(QCursor::pos());
}


void MainWindow::rm()
{
    QModelIndexList indexList = ui->treeView->selectionModel()->selectedRows();
    if (indexList.count() == 0) {
        return;
    }
    bool ok;
    for(int i=0;i<indexList.count();i++)
    {
        QModelIndex index = indexList.at(i);
        if (dirModel->fileInfo(index).isDir())
        {
            ok = dirModel->rmdir(index);
        } else {
            ok = dirModel->remove(index);
        }
        if (!ok) {
//            QMessageBox::information(this,
//                                     tr("Remove"),
//                                     tr("Failed to remove %1").arg(dirModel->fileName(indexList.at(i))));
        }
    }
}
//本地目录刷新
void MainWindow::localDirRefresh(){
    dirModel->refresh();
}

//按键事件
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //按回车键，默认执行“连接”按钮操作
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        on_connectButton_clicked();
}

