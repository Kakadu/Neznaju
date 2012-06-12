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

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QTcpServer>
#include <QtGui/QInputDialog>
#include <QtCore/QPair>

enum PluginStatus {
    ST_NONE, ST_SERVER, ST_CLIENT
};
enum CommandSort {
    CMD_FULL, CMD_ADD, CMD_DEL, CMD_UNKNOWN
};

static const int hotKeyListen  = Qt::CTRL + Qt::Key_I;
static const int hotKeyConnect = Qt::CTRL + Qt::Key_Y;

class NeznajuPluginView
    : public QObject, public KXMLGUIClient
{
    Q_OBJECT
    PluginStatus pluginStatus;
    KTextEditor::View *m_view;
    QTcpServer *server;
    QTcpSocket *clientSocket;
    int port;
    int server_status;
    QMap<int, QTcpSocket *> SClients;
    QString oldText;
    bool lockSend;

public:
    explicit NeznajuPluginView(KTextEditor::View *view = 0);
    ~NeznajuPluginView() {}
private:
    void sendToServer(QByteArray &);
    void sendToClients(QByteArray &, int clientId = -1);
    void fullText(const QString &str);
    void addText(QString str);
    void delText(QString str);
    void applyChanges(const QByteArray&);
    void transmitCommand(const QString&);

private Q_SLOTS:
    QPair<CommandSort, QString> splitHelper2(const QString& msg,
            int left, int& right);
    void slotStartServer();
    void onNewUserConnected();
    void fromClientReceived();
    void fromServerReceived();
    void sendFull(int clientId);
    void clientTryToConnect();
    void onDocumentTextInserted(KTextEditor::Document* doc, KTextEditor::Range rng);
    void onDocumentTextRemoved(KTextEditor::Document* doc, KTextEditor::Range rng);
private:
};

#endif // NEZNAJU_H
