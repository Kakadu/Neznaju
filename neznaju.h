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

#include "diff_match_patch.h"

enum PluginStatus {
    ST_NONE,ST_SERVER,ST_CLIENT
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
    void updateText(QString str);
    void addText(QString str);
    void delText(QString str);
    QString _oldText;
    //bool _fromServer;
    bool _isRemoteMessage;

  public:
    explicit NeznajuPluginView(KTextEditor::View *view = 0);
    ~NeznajuPluginView() {}

  private Q_SLOTS:
    void slotStartServer();
    void newUser();
    void fromClientReceived();
    void sendFull(int clientId);
    void transmitCommand(const QString&);
    void clientTryToConnect();
    void fromServerReceived();
    void sendToServer(QByteArray &);
    void sendToClients(QByteArray &, int clientId = -1);
    void documentTextInserted(KTextEditor::Document* doc,KTextEditor::Range rng);
    void documentTextRemoved(KTextEditor::Document* doc,KTextEditor::Range rng);
    void send(const QString &);
    void splitMessage(const QByteArray &str);
    void applyDiff(QString);
  private:
};

#endif // NEZNAJU_H
