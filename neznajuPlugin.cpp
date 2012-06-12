#include "neznajuPlugin.h"

NeznajuPlugin::NeznajuPlugin(QObject *parent, const QVariantList &args)
    : KTextEditor::Plugin(parent)
{
    Q_UNUSED(args);
}

NeznajuPlugin::~NeznajuPlugin()
{
}

void NeznajuPlugin::addView(KTextEditor::View *view)
{
    NeznajuPluginView *nview = new NeznajuPluginView(view);
    m_views.append(nview);
}

// Find the view where we want to remove the plugin from, and remove it.
// Do not forget to free the memory.
void NeznajuPlugin::removeView(KTextEditor::View *view)
{
    for (int z = 0; z < m_views.size(); z++) {
        if (m_views.at(z)->parentClient() == view) {
            NeznajuPluginView *nview = m_views.at(z);
            m_views.removeAll(nview);
            delete nview;
        }
    }
}

void NeznajuPlugin::readConfig()
{
}

void NeznajuPlugin::writeConfig()
{
}

