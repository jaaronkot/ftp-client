#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QWidget>
#include <QDirModel>
#include <QTreeView>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QkeyEvent>



#define SERVERPORT 8021

class QFtp;

#include <QHash>
class QFile;
class QUrlInfo;
class QTreeWidgetItem;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    QFtp *ftp;
    //初始化界面显示
    void initDisplay();
    // 用来存储FTP端一个路径是否为目录的信息
    QHash<QString, bool> isDirectory;
    //判断本地路径是否为目录
    QHash<QString, bool> isLocalDirectory;
    // 用来存储现在的路径
    QString currentPath;
    QDirModel *dirModel;
    //用来表示下载的文件
    QFile *file;
    void showLocalFile();
    //两个转码函数，解决中文名文件乱码
    QString _ToSpecialEncoding(const QString &InputStr);
    QString _FromSpecialEncoding(const QString &InputStr);
    //下载FTP端文件
    void downloadFtpFile(int rowIndex);
    void uploadLocalFile(int rowIndex);
    //客户端，服务器端treeview右键菜单
    QMenu *m_server_menu;
    QMenu *m_client_menu;

    //记录上传和下载文件的个数
    int indexCount;
    int currentIndex;

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void ftpCommandStarted(int);
    void ftpCommandFinished(int, bool);

    // 更新进度条
    void updateDataTransferProgress(qint64, qint64 );
    // 将服务器上的文件添加到Tree Widget部件中
    void addToList(const QUrlInfo &urlInfo);
    // 双击一个目录时显示其内容
    void processItem(QTreeWidgetItem*, int);
    void on_connectButton_clicked();
    void on_cdToParentButton_clicked();
    void on_downloadButton_clicked();
    //新建目录与删除操作
    void mkdir();
    void rm();

    void localDirRefresh();
    void on_treeView_doubleClicked(const QModelIndex &index);
    void on_uploadButton_clicked();
    //TreeView显示右键菜单
    void showFtpTreeViewMenu(const QPoint &point );
    void showLocalTreeViewMenu(const QPoint &point );

    void slotMkdir();
    void slotDeleteFile();
    void slotRefreshFtpList();
};

#endif // MAINWINDOW_H
