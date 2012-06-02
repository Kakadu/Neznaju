#ifndef NEZNAJU_H
#define NEZNAJU_H

#include <ktexteditor/plugin.h>
#include <ktexteditor/view.h>
#include <ktexteditor/range.h>
#include <ktexteditor/cursor.h>
#include <kxmlguiclient.h>
#include <klocalizedstring.h>

#include <QtCore/QEvent>
#include <QtCore/QObject>
#include <QtCore/QList>

#include <QtNetwork>
#include <QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtGui/QInputDialog>
#include <QtXml/QXmlReader>
#include <QtCore/QPair>

#include "diff_match_patch.h"

enum PluginStatus {
    ST_NONE,ST_SERVER,ST_CLIENT
};
enum CommandSort {
    CMD_FULL,CMD_ADD,CMD_DEL,CMD_UNKNOWN
};

    static const int _hotKeyListen  = Qt::CTRL + Qt::Key_I;
    static const int _hotKeyConnect = Qt::CTRL + Qt::Key_Y;

/**
  * This is the plugin view class. There can be as much instances as views exist.
  */
class NeznajuPluginView
   : public QObject, public KXMLGUIClient
{
  Q_OBJECT
    QTcpServer *_server;
    int _port;
    QTcpSocket *_clientSocket;
    diff_match_patch dmp;
    int server_status;
    PluginStatus _pluginStatus;
    QMap<int,QTcpSocket *> SClients;
    KTextEditor::View *m_view;
    QString _oldText;
    //bool _fromServer;
    bool _isRemoteMessage;

  public:
    explicit NeznajuPluginView(KTextEditor::View *view = 0);
    ~NeznajuPluginView() {}
  private:
    void sendToServer(QByteArray &);
    void sendToClients(QByteArray &, int clientId = -1);
    void fullText(QString str);
    void addText(QString str);
    void delText(QString str);

  private Q_SLOTS:
    // TODO: move methods which are not slots to private block
    QPair<CommandSort,QString> splitHelper2(const QString& msg,
                             int left,int& right);
    void slotStartServer();
    void onNewUserConnected();
    void applyChanges(const QByteArray&);
    void fromClientReceived();
    void fromServerReceived();
    void sendFull(int clientId);
    void transmitCommand(const QString&);
    void clientTryToConnect();
    void onDocumentTextInserted(KTextEditor::Document* doc,KTextEditor::Range rng);
    void onDocumentTextRemoved(KTextEditor::Document* doc,KTextEditor::Range rng);
    void send(const QString &);
    void splitMessage(const QByteArray &str);
    void applyDiff(QString);
  private:
};

#endif // NEZNAJU_H
