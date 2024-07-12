#include "server.h"

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
      file_infos_[info.fileName()] = serializable_info;
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
  QTcpSocket *sock = (QTcpSocket *)sender();

  quint8 signature;
  sock->read((char *)&signature, sizeof(quint8));

  if (signature == (char)'U') {
    AcceptFile(sock);
  } else if (signature == (char)'D') {
    UploadFile(sock);
  }
}

void Server::AcceptFile(QTcpSocket *sock) {
  QDataStream in(sock);
  in.setVersion(QDataStream::Qt_5_0);

  QString filename;
  in >> filename;

  qint64 size;
  in >> size;

  QFile file = files_.filePath(filename);
  if (file.exists()) {
    SendError(tr("File with name '%1' already exists").arg(filename), sock);
    return;
  }

  qint64 received = 0;
  file.open(QIODevice::WriteOnly);
  while (received < size) {
    qint64 remainder = size - received;
    qint64 to_read = remainder < 8192 ? remainder : 8192;
    QByteArray buf = sock->read(to_read);
    received += file.write(buf);
    if (buf.isEmpty()) {
      if (!sock->waitForReadyRead()) {
        break;
      }
    }
  }

  QFileInfo qinfo(file);
  FileInfo info = {.name = filename,
                   .unix_time = qinfo.birthTime().toSecsSinceEpoch()};

  file.close();
  AddNewFile(info);

  SendOk(sock);
  SendTableDataToAll();
}

void Server::UploadFile(QTcpSocket *sock) {
  QDataStream in(sock);
  in.setVersion(QDataStream::Qt_5_0);

  QString filename;
  in >> filename;

  if (!file_infos_.contains(filename)) {
    SendError(tr("Filename '%1' doesn't exist").arg(filename), sock);
    return;
  }

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  QString filepath = files_.absoluteFilePath(filename);
  QFile file(filepath);
  QFileInfo info(file);

  qint64 size = info.size();
  out << (quint8)'F';
  out << (qint64)size;
  file.open(QIODevice::ReadOnly);
  sock->write(block);
  sock->waitForBytesWritten();

  qint64 sent = 0;
  while (sent < size) {
    QByteArray buf = file.read(8192);
    sent += sock->write(buf.data(), buf.size());
    sock->waitForBytesWritten();
  }

  file.close();
}

void Server::SendError(const QString &message, QTcpSocket *sock) {
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  out << (quint8)'E';
  out << message;

  sock->write(block);
  sock->waitForBytesWritten();
}

void Server::SendOk(QTcpSocket *sock) {
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  out << (quint8)'O';

  sock->write(block);
  sock->waitForBytesWritten();
}

void Server::AddNewFile(FileInfo info) {
  file_infos_[info.name] = info;
  SendTableDataToAll();
}
