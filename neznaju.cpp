// Own includes
#include "neznaju.h"

// Include the basics
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
K_PLUGIN_FACTORY(TimeDatePluginFactory,
                 registerPlugin<TimeDatePlugin>();
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
K_EXPORT_PLUGIN(TimeDatePluginFactory("ktexteditor_neznaju", "ktexteditor_plugins"))


// Plugin view class
NeznajuPluginView::NeznajuPluginView(KTextEditor::View *view)
  : QObject(view)
  , KXMLGUIClient(view)
  , m_view(view)
{
    setComponentData(TimeDatePluginFactory::componentData());

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
    _pluginStatus=ST_NONE;
    this->_server = new QTcpServer();
    connect(_server, SIGNAL(newConnection()), this, SLOT(newUser()));
    connect(m_view->document(), SIGNAL(textChanged(KTextEditor::Document*)),
            this, SLOT(documentChanged()) );
    connect(m_view->document(),SIGNAL(textInserted(KTextEditor::Document*,KTextEditor::Range)),
            this,SLOT(documentTextInserted(KTextEditor::Document*,KTextEditor::Range)  ));
    connect(m_view->document(),SIGNAL(textRemoved(KTextEditor::Document*,KTextEditor::Range)),
            this,SLOT(documentTextRemoved(KTextEditor::Document*,KTextEditor::Range)  ));

    dmp.diff_main("","");

    _oldText="";
    _fromServer=false;
}

void NeznajuPluginView::updateText(QString str){
    _fromServer=true;
    if (m_view->document()->text() != str) {

        KTextEditor::Cursor cur=m_view->cursorPosition();

        m_view->document()->replaceText(
                KTextEditor::Range(KTextEditor::Cursor(0,0),
                KTextEditor::Cursor(m_view->document()->lines(),
                      m_view->document()->line( m_view->document()->lines()).size()   ) ),str);

        m_view->setCursorPosition(cur);

        _oldText= m_view->document()->text();

        //m_view->document()->clear();
        //m_view->document()->insertText( m_view->cursorPosition(), str);
    }
}

void NeznajuPluginView::addText(QString str){
    _fromServer=true;
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

    //KTextEditor::Range(curs1,curs2);

    m_view->document()->insertText(curs1,str);

    //qDebug() << "Curline:" << cur.line() << str;
}

void NeznajuPluginView::delText(QString str){
    _fromServer=true;
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

    m_view->document()->removeText(rng);

    //qDebug() << "Curline:" << cur.line() << str;
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
    connect(_clientSocket, SIGNAL(readyRead()), this, SLOT(clientReceivedData()) );
}

void NeznajuPluginView::applyDiff(QString str2) {
    QString str=str2;

    if (str.startsWith("<add>")) {
        int n=str.indexOf("</add>");
        if (n!=-1) {
            str=str.mid(5,n-5);
            addText(str);
        }
    } else
    if (str.startsWith("<del>")) {
        int n=str.indexOf("</del>");
        if (n!=-1) {
            str=str.mid(5,n-5);
            delText(str);
        }
    } else
    if (str.startsWith("<change>")) {
        int n=str.indexOf("</change>");
        if (n!=-1) {
            str=str.mid(8,n-8);
            QList<Patch> lst = dmp.patch_fromText(str);
            QPair<QString, QVector<bool> > out
               = dmp.patch_apply(lst, m_view->document()->text());
            //TODO: check diff correctness
            updateText(out.first);
        }
    } else
    if (str.startsWith("<full>")) {
        int n=str.indexOf("</full>");
        if (n!=-1) {
            str=str.mid(6,n-6);
            updateText(str);
        }
    }
}

void NeznajuPluginView::clientReceivedData() {
    if (_clientSocket->bytesAvailable() <= 0)
        return;

    QByteArray str = _clientSocket->readAll();
    QString tmp;
    for (auto i:str)
        tmp +=  QString::number((int)i);
    qDebug() << "bytes: " << tmp;
    qDebug() << "message: "<< str;
    splitMessage(str);
}

static int findDelim(const QByteArray &str,int start = 0) {
    for (int i=start; i<str.length()-1; ++i) {
        // -1 because (signed char) (0xFF) == -1
        if (str[i]==-1 && str[i+1]==-1)
            return i+2;
    }
    return -1;
}

void NeznajuPluginView::splitMessage(const QByteArray &str) {
    int left = 0, right;
    QByteArray tmp;
    do {
        right = findDelim(str,left);
        tmp = str.mid(left,right-left);
        //qDebug() << "splitMsg " << tmp;
        applyDiff(QUrl::fromPercentEncoding(tmp));
        left = right;
    } while(left != -1);
}

void NeznajuPluginView::documentTextInserted(KTextEditor::Document* doc,KTextEditor::Range rng){
    //qDebug() << "Text added: " << rng.start().column() << "-" <<  rng.end().column() << doc->text(rng);
    if (_fromServer==true)
    {
        _fromServer=false;
        return;
    }

    QString str=QString("<add>%1,%2,%3,%4,%5</add>").arg(rng.start().line())
                .arg(rng.start().column())
                .arg(rng.end().line())
                .arg(rng.end().column())
                .arg(doc->text(rng));
    send(str);
}

void NeznajuPluginView::send(const QString &msg) {
    QByteArray arr = QUrl::toPercentEncoding(msg);
    arr.push_back(0xFF);
    arr.push_back(0xFF);
    QString tmp;
    for (auto i:arr)
        tmp +=  QString::number((int)i);
    qDebug() << "Sendbytes: " << tmp;

    if (_pluginStatus == ST_SERVER) {
        for (auto i=SClients.begin(); i!=SClients.end(); ++i) {
            (*i)->write(arr.data());
            (*i)->flush();
        }
    } else if (_pluginStatus == ST_CLIENT) {
        _clientSocket->write(arr.data() );
        _clientSocket->flush();
    }
}

void NeznajuPluginView::documentTextRemoved(KTextEditor::Document* doc,KTextEditor::Range rng){
    //qDebug() << "Text removed: " << rng.start().column() << "-" << rng.end().column();
    if (_fromServer==true)
    {
        _fromServer=false;
        return;
    }

    QString str=QString("<del>%1,%2,%3,%4</del>").arg(rng.start().line())
                .arg(rng.start().column())
                .arg(rng.end().line())
                .arg(rng.end().column());
    //qDebug() << str;
    send(str);
}

void NeznajuPluginView::documentChanged(){
    /*
    if (m_view->document()->text() == "")
        return;
    //m_view->document()->textInserted();
    if (_fromServer==true)
    {
        _fromServer=false;
        return;
    }

    QString str1=_oldText;
    QString str2=m_view->document()->text();

    if (str1==str2) return;


    QString patchStr =
            dmp.patch_toText(dmp.patch_make(str1,str2) );
    _oldText=m_view->document()->text();

    if (_pluginStatus == ST_SERVER) {


        for (auto i=SClients.begin();i!=SClients.end();i++)  {
         //m_view->document()->text());

         (*i)->write(QString("<change>%1</change>").arg(patchStr)
                     .toUtf8().data() );

        }

    } else if (_pluginStatus == ST_CLIENT) {
        _clientSocket->write(QString("<change>%1</change>").arg(patchStr)
                             .toUtf8().data() );
    }
*/
}

void NeznajuPluginView::newUser() {
    qDebug() << "inside newUser()";
    if(server_status==1) { // TODO: What this variable means
         QTcpSocket* clientSocket = _server->nextPendingConnection();
         int idusersocs = clientSocket->socketDescriptor();
         qDebug() << "We have new connection: " << idusersocs;
         SClients[idusersocs]=clientSocket;
         connect(SClients[idusersocs],SIGNAL(readyRead()),this, SLOT(readClient()));

         QTextStream os(clientSocket);
         os.setAutoDetectUnicode(true);
         os << "<full>"+m_view->document()->text() +"</full>";
     }
}

void NeznajuPluginView::readClient() {
     QTcpSocket* clientSocket = (QTcpSocket*)sender();
     QByteArray str = clientSocket->readAll();
     splitMessage(str);
     //m_view->document()->insertText(m_view->cursorPosition(), str+"\n");
     //ui->textinfo->append("ReadClient:"+clientSocket->readAll()+"\n\r");
     // Если нужно закрыть сокет
     //clientSocket->close();
     //SClients.remove(idusersocs);
}
/*
void NeznajuPluginView::onTimer() {
    m_view->document()->insertText(m_view->cursorPosition(), "Wazza!");
}
*/
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
        server_status=1;
        qDebug() << QString::fromUtf8("Сервер запущен!");
        _pluginStatus=ST_SERVER;
    }
/*
    QString localizedTimeDate = i18nc("This is a localized string for default time & date printing on kate document."
                                      "%e means day in XX format."
                                      "%m means month in XX format."
                                      "%Y means year in XXXX format."
                                      "%H means hours in XX format."
                                      "%M means minutes in XX format."
                                      "Please, if in your language time or date is written in a different order, change it here",
                                      "%m-%e-%Y %H:%M");
*/
    // We create a KDateTime object with the current time & date.
    //KDateTime dt(QDateTime::currentDateTime());
    // We insert the information in the document at the current cursor position
    // with the default string declared on the header.
    //m_view->document()->insertText(m_view->cursorPosition(), dt.toString("HF GL!"));
}

// We need to include the moc file since we have declared slots and we are using
// the Q_OBJECT macro on the NeznajuPluginView class.
#include "neznaju.moc"
