#include "client.h"

#include <QAbstractSocket>
#include <QDataStream>
#include <QIODevice>
#include <QObject>
#include <QPushButton>
#include <QTableWidget>
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

  QObject::connect(sock_, &QAbstractSocket::errorOccurred, this,
                   &View::HandleError);
  QObject::connect(sock_, &QAbstractSocket::connected, this, &View::Connected);
  QObject::connect(sock_, &QIODevice::readyRead, this, &View::ReadTableData);

  in_.setDevice(sock_);
  in_.setVersion(QDataStream::Qt_5_0);
}

void View::Connect() { sock_->connectToHost("127.0.0.1", 1512); }

void View::Connected() {
  connect_btn_->setDisabled(true);
  connect_btn_->setText("connected");
}

void View::ReadTableData() {
  in_.startTransaction();

  while (!in_.atEnd()) {
    FileInfo test;
    in_ >> test.name >> test.unix_time;
    qDebug() << test.name << ": " << test.unix_time;
  }

  in_.commitTransaction();
}

void View::HandleError(QAbstractSocket::SocketError error) {
  qDebug() << error;
  // TODO implement actual error handling
}
