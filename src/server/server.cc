#include "server.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QHostAddress>
#include <QMessageBox>
#include <QString>
#include <QTcpServer>
#include <QTimeZone>
#include <QWidget>

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
    return false;
  }

  return true;
}

bool Server::InitFilesFolder(const QString &files_path) {
  QDir dir = QDir::current();
  if (!dir.mkdir(files_path) && !dir.exists(files_path)) {
    QMessageBox::critical(
        this, tr("File Transfer Server"),
        tr("Unable to start the server: %1.").arg("Couldn't make files dir"));
    return false;
  }

  if (!dir.cd(files_path)) {
    QMessageBox::critical(this, tr("File Transfer Server"),
                          tr("Unable to start the server: %1.")
                              .arg("Couldn't move to files dir"));
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
