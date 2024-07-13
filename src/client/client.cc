#include "client.h"

#include <QAbstractItemView>
#include <QAbstractSocket>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QIODevice>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTableWidgetSelectionRange>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QtGlobal>
#include <iostream>

// TODO remove this
#include <QDebug>

View::View(QWidget *parent)
    : QWidget(parent), main_table_(new QTableWidget(this)),
      main_layout_(new QVBoxLayout(this)),
      button_layout_(new QHBoxLayout(this)), sock_(new QTcpSocket(this)),
      connect_btn_(new QPushButton("connect", this)),
      upload_btn_(new QPushButton("upload", this)),
      download_btn_(new QPushButton("download", this)),
      state_(ClientState::kDisconnected) {
  main_layout_->addWidget(main_table_);
  main_layout_->addLayout(button_layout_);

  button_layout_->addWidget(connect_btn_);
  button_layout_->addWidget(upload_btn_);
  button_layout_->addWidget(download_btn_);

  setLayout(main_layout_);
  QObject::connect(connect_btn_, &QPushButton::pressed, this, &View::Connect);
  QObject::connect(upload_btn_, &QPushButton::pressed, this, &View::UploadFile);
  QObject::connect(download_btn_, &QPushButton::pressed, this,
                   &View::QueryDownloadFile);

  ConnectSocketSignals();

  in_.setDevice(sock_);
  in_.setVersion(QDataStream::Qt_5_0);

  main_table_->setColumnCount(3);
  main_table_->setHorizontalHeaderLabels({"Name", "Link", "Upload Date"});
  main_table_->setSelectionMode(QAbstractItemView::SingleSelection);
  QObject::connect(main_table_, &QTableWidget::itemSelectionChanged, this,
                   &View::HighlightDownloadButton);

  SetButtonsDisconnectedState();
}

void View::ConnectSocketSignals() {
  QObject::connect(sock_, &QAbstractSocket::errorOccurred, this,
                   &View::HandleError);
  QObject::connect(sock_, &QAbstractSocket::connected, this, &View::Connected);
  QObject::connect(sock_, &QAbstractSocket::disconnected, this,
                   &View::Disconnected);
  QObject::connect(sock_, &QIODevice::readyRead, this, &View::ParseMessage);
}

void View::Connect() { sock_->connectToHost("127.0.0.1", 1512); }

void View::Connected() {
  SetButtonsConnectedState();
  state_ = ClientState::kConnected;
}

void View::Disconnected() {
  SetButtonsDisconnectedState();
  state_ = ClientState::kDisconnected;
}

void View::ParseMessage() {
  qDebug() << "started parsing message";

  quint8 signature;
  in_ >> signature;

  switch (signature) {
  case 'T':
    qDebug() << "read table data";
    ReadTableData();
    break;
  case 'F':
    qDebug() << "download started";
    DownloadFile();
    break;
  case 'E':
    ParseError();
    break;
  default:
    std::cerr << "Unknown signature " << (char)signature << std::endl;
    break;
  }
}

void View::ReadTableData() {
  main_table_->clear();

  quint32 row_count;
  in_ >> row_count;
  qDebug() << row_count;

  main_table_->setRowCount(row_count);
  for (quint32 i = 0; i < row_count; ++i) {
    FileInfo info;
    in_ >> info.name >> info.unix_time;
    AddTableRow(info, i);
    qDebug() << i + 1 << info.name << info.unix_time;
  }
}

void View::AddTableRow(FileInfo info, int index) {
  QTableWidgetItem *name = new QTableWidgetItem(info.name);
  QTableWidgetItem *link = new QTableWidgetItem("Link to a document");

  QDateTime date_time = QDateTime::fromSecsSinceEpoch(info.unix_time);
  QTableWidgetItem *date = new QTableWidgetItem(date_time.toString());

  Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  name->setFlags(flags);
  link->setFlags(flags);
  date->setFlags(flags);

  main_table_->setItem(index, 0, name);
  main_table_->setItem(index, 1, link);
  main_table_->setItem(index, 2, date);
}

void View::UploadFile() {
  QString filepath = QFileDialog::getOpenFileName(this, "Select file to send");
  if (filepath.isNull()) {
    return;
  }
  SetButtonsLoadingState();

  QFile file(filepath);
  file.open(QIODevice::ReadOnly);
  QFileInfo info(file);

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  qint64 size = info.size();
  out << (quint8)'U';
  out << info.fileName();
  out << size;
  sock_->write(block);
  sock_->waitForBytesWritten();

  qint64 sent = 0;
  while (sent < size) {
    QByteArray buf = file.read(8192);
    sent += sock_->write(buf.data(), buf.size());
    sock_->waitForBytesWritten();
  }
  sock_->waitForBytesWritten();

  file.close();

  SetButtonsConnectedState();
}

void View::QueryDownloadFile() {
  if (selected_item_.isNull()) {
    return;
  }

  QString filepath = QFileDialog::getSaveFileName(this, "Select file location");
  if (filepath.isNull()) {
    return;
  }
  download_place_ = filepath;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  out << (quint8)'D';
  out << selected_item_;
  sock_->write(block);
  sock_->waitForBytesWritten();
  qDebug() << "asked for download";
}

void View::DownloadFile() {
  qint64 size;
  in_ >> size;
  qDebug() << "got size for file" << size;

  QFile file(download_place_);
  file.open(QIODevice::WriteOnly);

  qint64 received = 0;
  while (received < size) {
    qint64 remainder = size - received;
    qint64 to_read = remainder < 8192 ? remainder : 8192;
    QByteArray buf = sock_->read(to_read);
    received += file.write(buf);
    if (buf.isEmpty()) {
      if (!sock_->waitForReadyRead()) {
        break;
      }
    }
  }

  qDebug() << "downloaded";
  QMessageBox::information(this, "Success", "Download Finished");
}

void View::SetButtonsLoadingState() {
  connect_btn_->setDisabled(true);
  upload_btn_->setDisabled(true);
  download_btn_->setDisabled(true);
}

void View::SetButtonsDisconnectedState() {
  connect_btn_->setDisabled(false);
  upload_btn_->setDisabled(true);
  download_btn_->setDisabled(true);
}

void View::SetButtonsConnectedState() {
  connect_btn_->setDisabled(true);
  upload_btn_->setDisabled(false);
  download_btn_->setDisabled(true);
}

void View::HighlightDownloadButton() {
  if (state_ == ClientState::kDisconnected) {
    return;
  }

  QList<QTableWidgetSelectionRange> ranges = main_table_->selectedRanges();
  if (ranges.isEmpty()) {
    return;
  }
  QTableWidgetSelectionRange range = ranges[0];
  int col = range.leftColumn();
  int row = range.topRow();

  if (col == 1) {
    download_btn_->setEnabled(true);
    selected_item_ = main_table_->item(row, 0)->text();
  } else {
    download_btn_->setEnabled(false);
    selected_item_ = QString();
  }
}

void View::HandleError(QAbstractSocket::SocketError error) {
  (void)error;
  QMessageBox::critical(this, "Socket Error!", sock_->errorString());
}

void View::ParseError() {
  QString message;
  in_ >> message;

  QMessageBox::critical(this, "Server error", message);
}
