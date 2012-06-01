#ifndef NEZNAJUPLUGIN_H
#define NEZNAJUPLUGIN_H
#include "neznaju.h"

/**
  * This is the plugin class. There will be only one instance of this class.
  * We always want to inherit KTextEditor::Plugin here.
  */
class NeznajuPlugin
  : public KTextEditor::Plugin
{
  public:
    // Constructor
    explicit NeznajuPlugin(QObject *parent,
                            const QVariantList &args);
    // Destructor
    virtual ~NeznajuPlugin();

    // Overriden methods
    // This method is called when a plugin has to be added to a view. As there
    // is only one instance of this plugin, but it is possible for plugins to
    // behave in different ways in different opened views where it is loaded, in
    // Kate plugins are added to views. For that reason we have the plugin itself
    // (this class) and then the plugin view class.
    // In this methods we have to create/remove NeznajuPluginView classes.
    void addView (KTextEditor::View *view);
    void removeView (KTextEditor::View *view);

    void readConfig();
    void writeConfig();

  private:
    QList<class NeznajuPluginView*> m_views;
};

#endif // NEZNAJUPLUGIN_H
