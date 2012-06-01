#include "neznajuPlugin.h"

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
    NeznajuPluginView *nview = new NeznajuPluginView(view);
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
            NeznajuPluginView *nview = m_views.at(z);
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

