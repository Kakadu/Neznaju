// Own includes
#include "neznaju.h"

#include <ktexteditor/document.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdatetime.h>
#include <QtCore/QTimer>

#include "neznajuPlugin.h"

// This macro defines a KPluginFactory subclass named TimeDatePluginFactory. The second
// argument to the macro is code that is inserted into the constructor of the class.
// I our case all we need to do is register one plugin. If you want to have more
// than one plugin in the same library then you can register multiple plugin classes
// here. The registerPlugin function takes an optional QString parameter which is a
// keyword to uniquely identify the plugin then (it maps to X-KDE-PluginKeyword in the
// .desktop file).
K_PLUGIN_FACTORY(NeznajuPluginFactory,
                 registerPlugin<NeznajuPlugin>();
                )

// With the next macro call, the library exports version information about the
// Qt and KDE libraries being used and (most important) the entry symbol to get at
// the factory we defined above.
// The argument this macro takes is the constructor call of the above factory which
// provides two constructors. One which takes a KAboutData* and another one
// that takes two (optional) const char* parameters (Same as for KComponentData
// constructors).
// We put there the X-KDE-LibraryName.
// Is important to provide as last parameter "ktexteditor_plugins".
K_EXPORT_PLUGIN(NeznajuPluginFactory("ktexteditor_neznaju", "ktexteditor_plugins"))


// Plugin view class
NeznajuPluginView::NeznajuPluginView(KTextEditor::View *view)
  : QObject(view)
  , KXMLGUIClient(view)
  , m_view(view)
{
    setComponentData(NeznajuPluginFactory::componentData());

    KAction *action = new KAction(i18n("Insert Time && Date"), this);
    // Here we need as first parameter the same we declared at the resource
    // contents file (neznajuui.rc). We named the action "tools_insert_neznaju".
    // Here is where we connect it to an actual KDE action.

    actionCollection()->addAction("tools_insert_neznaju", action);
    action->setShortcut(_hotKeyListen);
    connect(action, SIGNAL(triggered()), this, SLOT(slotStartServer()));

    KAction *action2 = new KAction(i18n("connect_to_server"), this);
    actionCollection()->addAction("tools_client_connect", action2);
    action2->setShortcut(_hotKeyConnect);
    connect(action2, SIGNAL(triggered()), this, SLOT(clientTryToConnect()) );

    // This is always needed, tell the KDE XML GUI client that we are using
    // that file for reading actions from.
    setXMLFile("neznajuui.rc");
    _pluginStatus = ST_NONE;
    _server = new QTcpServer();
    connect(_server, SIGNAL(newConnection()), this, SLOT(onNewUserConnected()));
    connect(m_view->document(),SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)),
            this,SLOT(onDocumentTextInserted(KTextEditor::Document*,KTextEditor::Range) ));
    connect(m_view->document(),SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)),
            this,SLOT(onDocumentTextRemoved(KTextEditor::Document*,KTextEditor::Range) ));

    _oldText="";
    _lockSend = false;
}

void NeznajuPluginView::fullText(const QString &str) {
    if (m_view->document()->text() != str) {
        _lockSend=true;
        KTextEditor::Cursor cur = m_view->cursorPosition();
        m_view->document()->setText(str);
        _lockSend=false;
        if (!m_view->setCursorPosition(cur))
            qDebug() << "Cant revert cursor position after setting full text";
    }
}

void NeznajuPluginView::addText(QString str){
    qDebug() << "Str:" << str;
    KTextEditor::Cursor curs1,curs2;
    int n;

    n=str.indexOf(",");
    curs1.setLine(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs1.setColumn(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs2.setLine(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs2.setColumn(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    _lockSend = true;
    m_view->document()->insertText(curs1,str);
    _lockSend = false;
}

void NeznajuPluginView::delText(QString str){
    qDebug() << "Str:" << str;
    KTextEditor::Cursor curs1,curs2;
    int n;

    n=str.indexOf(",");
    curs1.setLine(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs1.setColumn(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs2.setLine(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    n=str.indexOf(",");
    curs2.setColumn(str.left(n).toInt());
    str=str.mid(n+1,str.length());

    KTextEditor::Range rng(curs1,curs2);

    _lockSend = true;
    m_view->document()->removeText(rng);
    _lockSend = false;
}


void NeznajuPluginView::clientTryToConnect() {
    bool ok = false;
    QString serverInfo = QInputDialog::getText(NULL, QString("Enter server IP:Port"), QString("IP:Port"),
                                             QLineEdit::Normal, "127.0.0.1:3333", &ok);
    if (!ok) {
        qDebug() << "User don't wnat to connect";
        return;
    }
    int pos = serverInfo.indexOf(":");
    if (pos==-1) {
        qDebug() << "User have written bad data";
        return;
    }
    QString ip = serverInfo.left(pos), port = serverInfo.right(serverInfo.count()-pos-1);
    qDebug() << "ip = " << ip << "\nport  = " << port;

    this->_clientSocket = new QTcpSocket();

    _clientSocket->connectToHost(ip,port.toInt());

    _pluginStatus=ST_CLIENT;
    connect(_clientSocket, SIGNAL(readyRead()), this, SLOT(fromServerReceived()) );
}

void NeznajuPluginView::onDocumentTextInserted(KTextEditor::Document* doc,KTextEditor::Range rng){
    QString str = QString("<add>%1,%2,%3,%4,%5</add>")
                .arg(rng.start().line())
                .arg(rng.start().column())
                .arg(rng.end().line())
                .arg(rng.end().column())
                .arg(doc->text(rng));
    transmitCommand(str);
}

void NeznajuPluginView::onDocumentTextRemoved(KTextEditor::Document* doc,KTextEditor::Range rng){
    QString str = QString("<del>%1,%2,%3,%4</del>")
                .arg(rng.start().line())
                .arg(rng.start().column())
                .arg(rng.end().line())
                .arg(rng.end().column());
    transmitCommand(str);
}

void NeznajuPluginView::fromClientReceived() {
     QTcpSocket* clientSocket = (QTcpSocket*)sender();
     auto msg = clientSocket->readAll();
     applyChanges(msg);
     sendToClients(msg,clientSocket->socketDescriptor());
}

void NeznajuPluginView::fromServerReceived() {
    qDebug() << "fromServerReceived";
    if (_clientSocket->bytesAvailable() <= 0)
        return;

    applyChanges(_clientSocket->readAll());
}

QPair<CommandSort,QString> NeznajuPluginView::splitHelper2(const QString& msg,
                                     int left,int& right) {
    static QString ans;
    static auto fail = qMakePair(CMD_UNKNOWN,QString(""));
    if (msg.indexOf("<add>",left) == left) {
        right = msg.indexOf("</add>",left);
        if (right==-1)
            return fail;
        ans = msg.mid(left+5,right-left-5);
        right += 6;
        return qMakePair(CMD_ADD, ans);
    } else if (msg.indexOf("<del>",left) == left) {
        right = msg.indexOf("</del>",left);
        if (right==-1)
            return fail;
        ans = msg.mid(left+5,right-left-5);
        right += 6;
        return qMakePair(CMD_DEL, ans);
    } else if (msg.indexOf("<full>",left) == left) {
        right = msg.indexOf("</full>",left);
        if (right==-1)
            return fail;
        ans = msg.mid(left+6,right-left-6);
        right += 7;
        return qMakePair(CMD_FULL, ans);
    } else
        return fail;

}
void NeznajuPluginView::applyChanges(const QByteArray& msg1) {
    qDebug() << "applyChanges";
    QString msg = QUrl::fromPercentEncoding(msg1);
    int left=0, right;
    do {
        QPair<CommandSort,QString> typ = splitHelper2(msg,left,right);
        switch (typ.first) {
        case CMD_ADD:
            addText(typ.second);
            left = right;
            continue;
        case CMD_DEL:
            delText(typ.second);
            left = right;
            continue;
        case CMD_FULL:
            fullText(typ.second);
            left = right;
            continue;
        case CMD_UNKNOWN:
            qDebug() << "UNKNOWN command received";
            break;
        }
        Q_ASSERT_X(false,"splitMessage2","Not all cases are in switch");
    } while (left < msg.count());
}

void NeznajuPluginView::transmitCommand(const QString &msg) {
    if(_lockSend) {
        qDebug() << "locksend = " << _lockSend;
        return;
    }
    QByteArray arr = QUrl::toPercentEncoding(msg);
    if (_pluginStatus == ST_SERVER)
        sendToClients(arr);
    else if (_pluginStatus == ST_CLIENT)
        sendToServer(arr);
}

void NeznajuPluginView::sendToServer(QByteArray &msg) {
    _clientSocket->write(msg);
    _clientSocket->flush();
}

void NeznajuPluginView::sendToClients(QByteArray &msg, int clientId) {
    qDebug() << "sendToClients " << msg;
    for (QMap<int,QTcpSocket *>::Iterator i=SClients.begin(); i!=SClients.end(); ++i)
        if (clientId != i.key()) {
            (*i)->write(msg.data());
            (*i)->flush();
        }
}

void NeznajuPluginView::onNewUserConnected() {
    if(server_status==1) { // TODO: What this variable means
         QTcpSocket* clientSocket = _server->nextPendingConnection();
         int idusersocs = clientSocket->socketDescriptor();
         qDebug() << "We have new connection: " << idusersocs;
         SClients[idusersocs] = clientSocket;
         connect(SClients[idusersocs],SIGNAL(readyRead()),
                 this, SLOT(fromClientReceived()));
         sendFull(idusersocs);
         qDebug() << "Sending full";
     }
}

void NeznajuPluginView::sendFull(int clientId) {
    QTextStream os(SClients[clientId]);
    QString msg = "<full>"+m_view->document()->text() +"</full>";
    os.setAutoDetectUnicode(true);
    os << QUrl::toPercentEncoding(msg).data();
}

void NeznajuPluginView::slotStartServer() {
    bool ok;
    int port = QInputDialog::getInt(NULL, QString("Enter the port"), QString("Port number"),
                                    3333, 1025, 65535,1, &ok);
    if (!ok) {
        qDebug() << "User dont want to connect";
        return;
    }

    if (!_server->listen(QHostAddress::Any, port) && server_status==0) {
        qDebug() << QObject::tr("Unable to start the server: %1.")
                    .arg(_server->errorString());
    } else {
        server_status = 1;
        _lockSend = false;
        qDebug() << QString::fromUtf8("Сервер запущен!");
        _pluginStatus=ST_SERVER;
    }
}

#include "neznaju.moc"
