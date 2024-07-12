#include "download_thread.h"
#include "server.h"
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QTcpSocket>
#include <QtGlobal>

DownloadThread::DownloadThread(QDataStream &in, qintptr sock_fd, QDir files,
                               QObject *parent)
    : QThread(parent), sock_fd_(sock_fd), in_(in), files_(files) {}

void DownloadThread::run() {
  QString filename;
  in_ >> filename;

  QFile file = files_.filePath(filename);
  if (file.exists()) {
    error_ = tr("File with name '%1' already exists").arg(filename);
    Finish();
    return;
  }

  QByteArray data;
  in_ >> data;
  in_.commitTransaction();

  file.open(QIODevice::WriteOnly);
  file.write(data);

  QFileInfo qinfo(file);
  FileInfo info = {.name = filename,
                   .unix_time = qinfo.birthTime().toSecsSinceEpoch()};

  file.close();
  emit NewFileAdded(info);

  Finish();
}

void DownloadThread::Finish() {
  QTcpSocket *sock = new QTcpSocket;
  sock->setSocketDescriptor(sock_fd_);

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  if (error_.isNull()) {
    out << (quint8)'O';
  } else {
    out << (quint8)'E';
    out << error_;
  }

  sock->write(block);
  sock->waitForBytesWritten(-1);
  sock->deleteLater();
}
