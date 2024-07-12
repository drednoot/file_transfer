#include "client.h"

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
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QWidget>
#include <Qt>
#include <QtGlobal>

// TODO remove this
#include <QDebug>

View::View(QWidget *parent)
    : QWidget(parent), main_table_(new QTableWidget(this)),
      main_layout_(new QVBoxLayout(this)),
      button_layout_(new QHBoxLayout(this)), sock_(new QTcpSocket(this)),
      connect_btn_(new QPushButton("connect", this)),
      upload_btn_(new QPushButton("upload", this)) {
  main_layout_->addWidget(main_table_);
  main_layout_->addLayout(button_layout_);

  button_layout_->addWidget(connect_btn_);
  button_layout_->addWidget(upload_btn_);

  setLayout(main_layout_);
  QObject::connect(connect_btn_, &QPushButton::pressed, this, &View::Connect);
  QObject::connect(upload_btn_, &QPushButton::pressed, this, &View::UploadFile);

  ConnectSocketSignals();

  in_.setDevice(sock_);
  in_.setVersion(QDataStream::Qt_5_0);

  main_table_->setColumnCount(3);
  main_table_->setHorizontalHeaderLabels({"Name", "Link", "Upload Date"});

  SetButtonsDefaultState();
}

void View::ConnectSocketSignals() {
  QObject::connect(sock_, &QAbstractSocket::errorOccurred, this,
                   &View::HandleError);
  QObject::connect(sock_, &QAbstractSocket::connected, this, &View::Connected);
  QObject::connect(sock_, &QAbstractSocket::disconnected, this,
                   &View::Disconnected);
  QObject::connect(sock_, &QIODevice::readyRead, this, &View::ParseMessage);
  QObject::connect(sock_, &QIODevice::bytesWritten, this,
                   &View::SetButtonsConnectedState);
}

void View::Connect() { sock_->connectToHost("127.0.0.1", 1512); }

void View::Connected() { SetButtonsConnectedState(); }

void View::Disconnected() { SetButtonsDefaultState(); }

void View::ParseMessage() {
  in_.startTransaction();

  quint8 signature;
  in_ >> signature;

  switch (signature) {
  case 'T':
    ReadTableData();
    break;
  case 'F':
    // TODO
    qDebug() << "Operation was not yet implemented";
    break;
  case 'E':
    // TODO
    ParseError();
    break;
  case 'O':
    // TODO
    qDebug() << "All OK";
    break;
  default:
    qDebug() << "Unknown operation";
    break;
  }

  in_.commitTransaction();
}

void View::ReadTableData() {
  main_table_->clear();

  quint32 row_count;
  in_ >> row_count;

  main_table_->setRowCount(row_count);
  for (quint32 i = 0; i < row_count; ++i) {
    FileInfo info;
    in_ >> info.name >> info.unix_time;
    AddTableRow(info, i);
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
  QFile file(filepath);
  file.open(QIODevice::ReadOnly);
  QFileInfo info(file);

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_0);

  out << (quint8)'F';
  out << info.fileName();

  out << file.readAll();
  file.close();

  sock_->write(block);
  SetButtonsUploadingState();
}

void View::SetButtonsUploadingState() {
  connect_btn_->setDisabled(true);
  upload_btn_->setDisabled(true);

  connect_btn_->setText("connect");
  upload_btn_->setText("uploading...");
}

void View::SetButtonsDefaultState() {
  connect_btn_->setDisabled(false);
  upload_btn_->setDisabled(true);

  connect_btn_->setText("connect");
  upload_btn_->setText("upload");
}

void View::SetButtonsConnectedState() {
  connect_btn_->setDisabled(true);
  upload_btn_->setDisabled(false);

  connect_btn_->setText("connected");
  upload_btn_->setText("upload");
}

void View::HandleError(QAbstractSocket::SocketError error) {
  qDebug() << error;
  // TODO implement actual error handling
}

void View::ParseError() {
  QString message;
  in_ >> message;

  // TODO qmessagebox
  qDebug() << message;
}
