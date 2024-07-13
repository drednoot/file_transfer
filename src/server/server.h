#ifndef FILE_TRANSFER_SERVER_SERVER_H_
#define FILE_TRANSFER_SERVER_SERVER_H_

#include <QDataStream>
#include <QDir>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWidget>
#include <QtGlobal>

struct FileInfo {
  QString name;
  qint64 unix_time;
};
Q_DECLARE_METATYPE(FileInfo)

class Server : public QWidget {
  Q_OBJECT

public:
  explicit Server(QWidget *parent = nullptr, const int port = 1512,
                  const QString &files_path = "./files/");

private slots:
  void AddConnection();
  void RemoveFromConnected();
  void ParseMessage();

private:
  bool InitServer();
  bool InitFilesFolder(const QString &files_path);
  void InflateFiles();

  void SendTableData(QTcpSocket *sock);
  void SendTableDataToAll();
  void AddNewFile(FileInfo info);

  void AcceptFile(QTcpSocket *sock);
  void UploadFile(QTcpSocket *sock);
  void SendError(const QString &message, QTcpSocket *sock);

  QTcpServer *qtcp_serv_;
  const int port_;
  QDir files_;
  QList<QTcpSocket *> connected_;
  QMap<QString, FileInfo> file_infos_;
};

#endif // FILE_TRANSFER_SERVER_SERVER_H_
