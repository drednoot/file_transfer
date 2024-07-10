#ifndef FILE_TRANSFER_SERVER_SERVER_H_
#define FILE_TRANSFER_SERVER_SERVER_H_

#include <QDir>
#include <QList>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWidget>
#include <QtGlobal>

class Server : public QWidget {
  Q_OBJECT

public:
  struct FileInfo {
    QString name;
    qint64 unix_time;
  };

  explicit Server(QWidget *parent = nullptr, const int port = 1512,
                  const QString &files_path = "./files/");

private:
  bool InitServer();
  bool InitFilesFolder(const QString &files_path);
  void InflateFiles();

  QTcpServer *qtcp_serv_;
  const int port_;
  QDir files_;
  QList<QTcpSocket *> connected_;
  QList<FileInfo> file_infos_;
};

#endif // FILE_TRANSFER_SERVER_SERVER_H_
