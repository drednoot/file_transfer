#include "client.h"

#include <QAbstractSocket>
#include <QDataStream>
#include <QDateTime>
#include <QIODevice>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QWidget>

// TODO remove this
#include <QDebug>

View::View(QWidget *parent)
    : QWidget(parent), main_table_(new QTableWidget(this)),
      main_layout_(new QVBoxLayout(this)), sock_(new QTcpSocket(this)),
      connect_btn_(new QPushButton("connect", this)) {
  main_layout_->addWidget(main_table_);
  main_layout_->addWidget(connect_btn_);

  setLayout(main_layout_);
  QObject::connect(connect_btn_, &QPushButton::pressed, this, &View::Connect);

  ConnectSocketSignals();

  in_.setDevice(sock_);
  in_.setVersion(QDataStream::Qt_5_0);

  main_table_->setColumnCount(3);
  main_table_->setHorizontalHeaderLabels({"Name", "Link", "Upload Date"});
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
  connect_btn_->setDisabled(true);
  connect_btn_->setText("connected");
}

void View::Disconnected() {
  connect_btn_->setDisabled(false);
  connect_btn_->setText("connect");
}

void View::ParseMessage() {
  in_.startTransaction();

  quint8 signature;
  in_ >> signature;
  qDebug() << "signature:" << (char)signature;

  switch (signature) {
  case 'T':
    ReadTableData();
    break;
  case 'D':
    // TODO
    qDebug() << "Operation was not yet implemented";
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
  qDebug() << "row count" << row_count;
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

  main_table_->setItem(index, 0, name);
  main_table_->setItem(index, 1, link);
  main_table_->setItem(index, 2, date);
}

void View::HandleError(QAbstractSocket::SocketError error) {
  qDebug() << error;
  // TODO implement actual error handling
}
