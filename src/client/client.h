#ifndef FILE_TRANSFER_CLIENT_CLIENT_H_
#define FILE_TRANSFER_CLIENT_CLIENT_H_

#include <QAbstractSocket>
#include <QDataStream>
#include <QHBoxLayout>
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

  enum ClientState {
    kDisconnected,
    kConnected,
    kLoading,
  };

  explicit View(QWidget *parent = nullptr);

  void ConnectSocketSignals();

private slots:
  void Connect();
  void Connected();
  void Disconnected();
  void HandleError(QAbstractSocket::SocketError error);
  void UploadFile();
  void QueryDownloadFile();
  void HighlightDownloadButton();

private:
  void ParseMessage();
  void ReadTableData();
  void ParseError();
  void DownloadFile();

  void AddTableRow(FileInfo info, int index);

  void SetButtonsLoadingState();
  void SetButtonsDisconnectedState();
  void SetButtonsConnectedState();

  QTableWidget *main_table_;
  QVBoxLayout *main_layout_;
  QHBoxLayout *button_layout_;
  QTcpSocket *sock_;
  QPushButton *connect_btn_;
  QPushButton *upload_btn_;
  QPushButton *download_btn_;

  QDataStream in_;

  QString download_place_;
  QString selected_item_;

  ClientState state_;
};

#endif // FILE_TRANSFER_CLIENT_CLIENT_H_
