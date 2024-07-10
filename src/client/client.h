#ifndef FILE_TRANSFER_CLIENT_CLIENT_H_
#define FILE_TRANSFER_CLIENT_CLIENT_H_

#include <QAbstractSocket>
#include <QDataStream>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QTableWidget>
#include <QTcpSocket>
#include <QVBoxLayout>
#include <QWidget>

// struct ConnectionDetails {
//   QString host;
//   QString port;
// };

class View : public QWidget {
  Q_OBJECT

public:
  struct FileInfo {
    QString name;
    qint64 unix_time;
  };

  explicit View(QWidget *parent = nullptr);

  void Connect();

public slots:
  void HandleError(QAbstractSocket::SocketError error);

private slots:
  void Connected();

private:
  void ReadTableData();

  QTableWidget *main_table_;
  QVBoxLayout *main_layout_;
  QTcpSocket *sock_;
  QPushButton *connect_btn_;

  QDataStream in_;
};

#endif // FILE_TRANSFER_CLIENT_CLIENT_H_
