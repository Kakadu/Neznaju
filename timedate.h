// Avoid multiple header inclusion
#ifndef TIMEDATE_H
#define TIMEDATE_H

// Include the basics
#include <ktexteditor/plugin.h>
#include <ktexteditor/view.h>
#include <kxmlguiclient.h>
#include <klocalizedstring.h>

#include <QtCore/QEvent>
#include <QtCore/QObject>
#include <QtCore/QList>

#include <QtNetwork>
#include <QTcpSocket>
#include <QtNetwork/QTcpServer>

/**
  * This is the plugin class. There will be only one instance of this class.
  * We always want to inherit KTextEditor::Plugin here.
  */
class TimeDatePlugin
  : public KTextEditor::Plugin
{
  public:
    // Constructor
    explicit TimeDatePlugin(QObject *parent,
                            const QVariantList &args);
    // Destructor
    virtual ~TimeDatePlugin();

    // Overriden methods
    // This method is called when a plugin has to be added to a view. As there
    // is only one instance of this plugin, but it is possible for plugins to
    // behave in different ways in different opened views where it is loaded, in
    // Kate plugins are added to views. For that reason we have the plugin itself
    // (this class) and then the plugin view class.
    // In this methods we have to create/remove TimeDatePluginView classes.
    void addView (KTextEditor::View *view);
    void removeView (KTextEditor::View *view);

    void readConfig();
    void writeConfig();

  private:
    QList<class TimeDatePluginView*> m_views;
};

/**
  * This is the plugin view class. There can be as much instances as views exist.
  */
class TimeDatePluginView
   : public QObject, public KXMLGUIClient
{
  Q_OBJECT
    QTcpServer *server;
  public:
    explicit TimeDatePluginView(KTextEditor::View *view = 0);
    ~TimeDatePluginView();

  private Q_SLOTS:
    void slotInsertTimeDate();
    void onTimer();
    void newUser();
    void readClient();

  private:
    int server_status;
    QMap<int,QTcpSocket *> SClients;
    KTextEditor::View *m_view;
};

#endif // TIMEDATE_H
