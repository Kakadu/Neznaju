// Own includes
#include "timedate.h"

// Include the basics
#include <ktexteditor/document.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kdatetime.h>
#include <QtCore/QTimer>

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
K_EXPORT_PLUGIN(TimeDatePluginFactory("ktexteditor_timedate", "ktexteditor_plugins"))

// Constructor
TimeDatePlugin::TimeDatePlugin(QObject *parent, const QVariantList &args)
    : KTextEditor::Plugin(parent)
{
    // Avoid warning on compile time because of unused argument
    Q_UNUSED(args);
}

// Destructor
TimeDatePlugin::~TimeDatePlugin()
{
}

// Create the plugin view class and add it to the views list
void TimeDatePlugin::addView(KTextEditor::View *view)
{
    TimeDatePluginView *nview = new TimeDatePluginView(view);
    m_views.append(nview);
}

// Find the view where we want to remove the plugin from, and remove it.
// Do not forget to free the memory.
void TimeDatePlugin::removeView(KTextEditor::View *view)
{
    for (int z = 0; z < m_views.size(); z++)
    {
        if (m_views.at(z)->parentClient() == view)
        {
            TimeDatePluginView *nview = m_views.at(z);
            m_views.removeAll(nview);
            delete nview;
        }
    }
}

// We do nothing on this methods since our plugin is not configurable yet
void TimeDatePlugin::readConfig()
{
}

void TimeDatePlugin::writeConfig()
{
}


// Plugin view class
TimeDatePluginView::TimeDatePluginView(KTextEditor::View *view)
  : QObject(view)
  , KXMLGUIClient(view)
  , m_view(view)
{
    setComponentData(TimeDatePluginFactory::componentData());

    KAction *action = new KAction(i18n("Insert Time && Date"), this);
    // Here we need as first parameter the same we declared at the resource
    // contents file (timedateui.rc). We named the action "tools_insert_timedate".
    // Here is where we connect it to an actual KDE action.

    actionCollection()->addAction("tools_insert_timedate", action);
    action->setShortcut(_hotKeyConnect);
    connect(action, SIGNAL(triggered()), this, SLOT(slotStartServer()));

    // This is always needed, tell the KDE XML GUI client that we are using
    // that file for reading actions from.
    setXMLFile("timedateui.rc");

    this->_server = new QTcpServer();
    connect(_server, SIGNAL(newConnection()), this, SLOT(newUser()));
}

void TimeDatePluginView::newUser() {
    qDebug() << "inside newUser()";
    if(server_status==1){
         qDebug() << QString::fromUtf8("У нас новое соединение!");
         QTcpSocket* clientSocket = _server->nextPendingConnection();
         int idusersocs=clientSocket->socketDescriptor();
         SClients[idusersocs]=clientSocket;
         connect(SClients[idusersocs],SIGNAL(readyRead()),this, SLOT(readClient()));

         QTextStream os(clientSocket);
         os.setAutoDetectUnicode(true);
         os << "<full>"+m_view->document()->text() +"</full>";
     }
}

void TimeDatePluginView::readClient() {
     QTcpSocket* clientSocket = (QTcpSocket*)sender();
     //int idusersocs=clientSocket->socketDescriptor();

     QString str=clientSocket->readAll();

     if (str.startsWith("<full>")) {
         int n=str.indexOf("</full>");
         if (n!=-1) {
             str=str.mid(6,n-6);
             m_view->document()->clear();
             m_view->document()->insertText(m_view->cursorPosition(), str);
         }
     }

     //m_view->document()->insertText(m_view->cursorPosition(), str+"\n");
     //ui->textinfo->append("ReadClient:"+clientSocket->readAll()+"\n\r");
     // Если нужно закрыть сокет
     //clientSocket->close();
     //SClients.remove(idusersocs);
}
/*
void TimeDatePluginView::onTimer() {
    m_view->document()->insertText(m_view->cursorPosition(), "Wazza!");
}
*/
void TimeDatePluginView::slotStartServer() {
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
// the Q_OBJECT macro on the TimeDatePluginView class.
#include "timedate.moc"
