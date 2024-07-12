#include "server.h"
#include "download_thread.h"

#include <QByteArray>
#include <QCoreApplication>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHostAddress>
#include <QIODevice>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThread>
#include <QTimeZone>
#include <QWidget>
#include <QtGlobal>

// TODO remove this
#include <QDebug>

Server::Server(QWidget *parent, const int port, const QString &files_path)
    : QWidget(parent), port_(port) {
  if (!InitServer()) {
    return;
  }
  if (!InitFilesFolder(files_path)) {
    return;
  }
  InflateFiles();
}

bool Server::InitServer() {
  qtcp_serv_ = new QTcpServer(this);
  if (!qtcp_serv_->listen(QHostAddress::LocalHost, port_)) {
    QMessageBox::critical(
        this, tr("File Transfer Server"),
        tr("Unable to start the server: %1.").arg(qtcp_serv_->errorString()));
    QCoreApplication::quit();
    return false;
  }

  QObject::connect(qtcp_serv_, &QTcpServer::newConnection, this,
                   &Server::AddConnection);

  return true;
}

bool Server::InitFilesFolder(const QString &files_path) {
  QDir dir = QDir::current();
  if (!dir.mkdir(files_path) && !dir.exists(files_path)) {
    QMessageBox::critical(
        this, tr("File Transfer Server"),
        tr("Unable to start the server: %1.").arg("Couldn't make files dir"));
    QCoreApplication::quit();
    return false;
  }

  if (!dir.cd(files_path)) {
    QMessageBox::critical(this, tr("File Transfer Server"),
                          tr("Unable to start the server: %1.")
                              .arg("Couldn't move to files dir"));
    QCoreApplication::quit();
    return false;
  }

  files_ = QDir(std::move(dir));
  return true;
}

void Server::InflateFiles() {
  QFileInfoList server_files = files_.entryInfoList();

  for (int i = 0; i < server_files.size(); ++i) {
    QFileInfo info = server_files.at(i);
    FileInfo serializable_info = {
        .name = info.fileName(),
        .unix_time = info.birthTime().toSecsSinceEpoch(),
    };

    if (info.isFile()) {
      file_infos_.append(serializable_info);
    }
  }
}

void Server::AddConnection() {
  QTcpSocket *sock = qtcp_serv_->nextPendingConnection();

  connected_.append(sock);

  QObject::connect(sock, &QTcpSocket::disconnected, this,
                   &Server::RemoveFromConnected);
  QObject::connect(sock, &QTcpSocket::readyRead, this, &Server::ParseMessage);

  sock->waitForConnected();
  SendTableData(sock);
}

void Server::RemoveFromConnected() {
  QTcpSocket *snd = (QTcpSocket *)sender();

  connected_.removeOne(snd);

  snd->deleteLater();
}

void Server::SendTableData(QTcpSocket *sock) {
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  out << (quint8)'T';
  out << (quint32)file_infos_.size();
  for (auto info : file_infos_) {
    out << info.name << info.unix_time;
    qDebug() << info.name << info.unix_time;
  }

  sock->write(block);
  sock->waitForBytesWritten();
}

void Server::SendTableDataToAll() {
  for (auto sock : connected_) {
    SendTableData(sock);
  }
}

void Server::ParseMessage() {
  QTcpSocket *snd = (QTcpSocket *)sender();
  QDataStream in(snd);
  in.setVersion(QDataStream::Qt_5_0);
  in.startTransaction();

  quint8 signature;
  in >> signature;

  if (signature == (char)'F') {
    AcceptFile(in, snd->socketDescriptor());
  }
}

void Server::AcceptFile(QDataStream &in, qintptr sock_fd) {
  DownloadThread *thread = new DownloadThread(in, sock_fd, files_, this);
  QObject::connect(thread, &DownloadThread::NewFileAdded, this,
                   &Server::AddNewFile);
  QObject::connect(thread, &QThread::finished, thread, &QObject::deleteLater);
  thread->start();
}

void Server::AddNewFile(FileInfo info) {
  file_infos_.append(info);
  SendTableDataToAll();
}
