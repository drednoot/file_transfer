#ifndef FILE_TRANSFER_SERVER_DOWNLOAD_THREAD_H_
#define FILE_TRANSFER_SERVER_DOWNLOAD_THREAD_H_

#include "server.h"

#include <QDataStream>
#include <QDir>
#include <QObject>
#include <QString>
#include <QThread>

class DownloadThread : public QThread {
  Q_OBJECT

public:
  DownloadThread(QDataStream &in, qintptr sock_fd, QDir files,
                 QObject *parent = nullptr);
  void run() override;

signals:
  void NewFileAdded(FileInfo info);

private:
  void Finish();

  qintptr sock_fd_;
  QDataStream &in_;
  QString error_;
  QDir files_;
};

#endif // FILE_TRANSFER_SERVER_DOWNLOAD_THREAD_H_
