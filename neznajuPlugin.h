#ifndef NEZNAJUPLUGIN_H
#define NEZNAJUPLUGIN_H
#include "neznaju.h"

/**
  * This is the plugin class. There will be only one instance of this class.
  */
class NeznajuPlugin
    : public KTextEditor::Plugin
{
public:
    explicit NeznajuPlugin(QObject *parent,
                           const QVariantList &args);
    virtual ~NeznajuPlugin();

    void addView(KTextEditor::View *view);
    void removeView(KTextEditor::View *view);

    void readConfig();
    void writeConfig();

private:
    QList<class NeznajuPluginView*> m_views;
};

#endif // NEZNAJUPLUGIN_H
