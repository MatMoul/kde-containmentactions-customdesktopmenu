/*
 *   Copyright 2015 by MatMoul <matmoul on sourceforge.net>
 * 
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "launch.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QFileInfo>

#include <KDebug>
#include <KIcon>
#include <KMenu>
#include <KProcess>
#include <KServiceGroup>

#include <Plasma/DataEngine>
#include <Plasma/Containment>
#include <Plasma/Service>

DesktopCustomMenu::DesktopCustomMenu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
    , m_action(new QAction(this))
{
    m_menu = new KMenu();
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));

    m_action->setMenu(m_menu);
}

DesktopCustomMenu::~DesktopCustomMenu()
{
    delete m_menu;
}

void DesktopCustomMenu::init(const KConfigGroup &config)
{
    QString defMenuConfig = "favorites\n";
    defMenuConfig += "-\n";
    defMenuConfig += "submenu\t" + i18n("Applications") + "\tkde\n";
    defMenuConfig += "programs\n";
    defMenuConfig += "endsubmenu\n";
    defMenuConfig += "-\n";
    defMenuConfig += "#/usr/share/applications/kde4/dolphin.desktop\n";
    defMenuConfig += "-\n";
    defMenuConfig += "submenu\t" + i18n("System") + "\tconfigure-shortcuts\n";
    defMenuConfig += "programs\tSettingsmenu/\n";
    defMenuConfig += "programs\tSystem/\n";
    defMenuConfig += "endsubmenu\n";
    defMenuConfig += "submenu\t" + i18n("Exit") + "\tsystem-shutdown\n";
    defMenuConfig += i18n("Lock") + "\tsystem-lock-screen\tqdbus-qt4 org.kde.ksmserver /ScreenSaver Lock\n";
    defMenuConfig += i18n("Disconnect") + "\tsystem-log-out\tqdbus-qt4 org.kde.ksmserver /KSMServer logout -1 0 3\n";
    defMenuConfig += i18n("Switch User") + "\tsystem-switch-user\tqdbus-qt4 org.kde.krunner /App switchUser\n";
    defMenuConfig += "-\n";
    defMenuConfig += i18n("Sleep") + "\tsystem-suspend\tqdbus-qt4 org.freedesktop.PowerManagement /org/freedesktop/PowerManagement Suspend\n";
    defMenuConfig += i18n("Hibernate") + "\tsystem-suspend-hybernate\tqdbus-qt4 org.freedesktop.PowerManagement /org/freedesktop/PowerManagement Hibernate\n";
    defMenuConfig += "-\n";
    defMenuConfig += i18n("Restart") + "\tsystem-reboot\tqdbus-qt4 org.kde.ksmserver /KSMServer logout -1 1 3\n";
    defMenuConfig += i18n("Shut down") + "\tsystem-shutdown\tqdbus-qt4 org.kde.ksmserver /KSMServer logout -1 2 3\n";
    defMenuConfig += "endsubmenu\n";
    defMenuConfig += "-\n";
    defMenuConfig += "/usr/share/applications/kde4/ksysguard.desktop\n";
    defMenuConfig += "/usr/share/applications/kde4/org.kde.konsole.desktop\n";
    
    menuconfig.clear();
    menuconfig = config.readEntry("menuconfig", defMenuConfig);
}

QWidget* DesktopCustomMenu::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    
    ui.setupUi(widget);
    widget->setWindowTitle(i18n("Configure Menu"));
    ui.menuconfig->setPlainText(menuconfig);
    
    return widget;
}

void DesktopCustomMenu::configurationAccepted()
{
    menuconfig = ui.menuconfig->toPlainText();
}

void DesktopCustomMenu::save(KConfigGroup &config)
{
    config.writeEntry("menuconfig", menuconfig);
}

void DesktopCustomMenu::contextEvent(QEvent *event)
{
    makeMenu();
    m_menu->adjustSize();
    m_menu->exec(popupPosition(m_menu->size(), event));
}

QList<QAction *> DesktopCustomMenu::contextualActions()
{
    makeMenu();

    QList<QAction *> list;
    list << m_action;
    return list;
}

void DesktopCustomMenu::makeMenu()
{
    Plasma::DataEngine *apps = dataEngine("apps");
    if (!apps->isValid()) {
        return;
    }

    m_menu->clear();

    if (!menuconfig.isEmpty()) {
      QList<QMenu*> menuList;
      menuList.append(m_menu);
      QMenu* curMenu = m_menu;
      QStringList configLines = menuconfig.split( "\n", QString::SkipEmptyParts );
        foreach( QString cfgLine, configLines ) {
            if (!cfgLine.startsWith("#")) {
                if (cfgLine == "-") {
                    curMenu->addSeparator();
                } else if (cfgLine.endsWith(".desktop")) {
                    addApp(curMenu, cfgLine);
                } else if (cfgLine.startsWith("submenu")) {
                  QStringList cfgParts = cfgLine.split( "\t", QString::SkipEmptyParts );
                  if (cfgParts.size() == 3) {
                    curMenu = addMnu(curMenu, cfgParts[1], cfgParts[2]);
                    menuList.append(curMenu);
                  }
                } else if (cfgLine.startsWith("endsubmenu")) {
                    menuList.removeLast();
                    curMenu = menuList.last();
                } else if (cfgLine == "favorites") {
                  KConfig config("kickoffrc");
                  KConfigGroup favoritesGroup = config.group("Favorites");
                  QList<QString> favoriteList = favoritesGroup.readEntry("FavoriteURLs", QList<QString>());
                  foreach (const QString &source, favoriteList) {
                    addApp(curMenu, source);
                  }
                } else if (cfgLine.startsWith("programs")) {
                  QStringList cfgParts = cfgLine.split( "\t", QString::SkipEmptyParts );
                  if (cfgParts.size() > 1) {
                    addAppMnu(curMenu, cfgParts[1]);
                  } else {
                    addAppMnu(curMenu, "");
                  }
                } else {
                  QStringList cfgParts = cfgLine.split( "\t", QString::SkipEmptyParts );
                  if (cfgParts.size() == 3) {
                    addCmd(curMenu, cfgParts[0], cfgParts[1], cfgParts[2]);
                  }
                }
            }
        }
    }
}

bool DesktopCustomMenu::addCmd(QMenu *menu, const QString &txt, const QString &iconpath, const QString &cmd)
{
    QString text = txt;
    text.replace("&", "&&"); //escaping
    KIcon icon(iconpath);
    QAction *action = menu->addAction(icon, text);
    action->setData(cmd);
    return true;
}

QMenu* DesktopCustomMenu::addMnu(QMenu *menu, const QString &txt, const QString &iconpath)
{
    QString text = txt;
    text.replace("&", "&&"); //escaping
    KIcon icon(iconpath);
    QMenu *subMenu = menu->addMenu(icon, text);
    return subMenu;
}

bool DesktopCustomMenu::addApp(QMenu *menu, const QString &path)
{
    QFileInfo info(path);
    QString source = "kde4-" + info.fileName();
    Plasma::DataEngine::Data app = dataEngine("apps")->query(source);

    QString name = app.value("name").toString();
    if (name.isEmpty()) {
        source = info.fileName();
    }

    if (source == "---") {
        menu->addSeparator();
        return false;
    }

    app = dataEngine("apps")->query(source);

    if (!app.value("display").toBool()) {
        kDebug() << "hidden entry" << source;
        return false;
    }
    name = app.value("name").toString();
    if (name.isEmpty()) {
        kDebug() << "failed source" << source;
        return false;
    }

    name.replace("&", "&&"); //escaping
    KIcon icon(app.value("iconName").toString());

    if (app.value("isApp").toBool()) {
        QAction *action = menu->addAction(icon, name);
        action->setData(source);
    } else { //ooh, it's actually a group!
        QMenu *subMenu = menu->addMenu(icon, name);
        bool hasEntries = false;
        foreach (const QString &source, app.value("entries").toStringList()) {
            hasEntries = addApp(subMenu, source) || hasEntries;
        }

        if (!hasEntries) {
            delete subMenu;
            return false;
        }
    }
    return true;
}

bool DesktopCustomMenu::addAppMnu(QMenu *menu, const QString &path){
  KServiceGroup::Ptr root = KServiceGroup::group(path);
  if(root.isNull()){
      return false;
  }
  KServiceGroup::List list = root->entries(true, true, true);
  foreach (const KServiceGroup::SPtr &service, list){
    if (service->isSeparator()) {
      menu->addSeparator();
    } else if (service->property("DesktopEntryPath").toString().isEmpty()) {
      KServiceGroup::Ptr dir = KServiceGroup::group(service->name());
      if(dir->childCount()>0){
        KIcon icon(dir->icon());
        QMenu* menu2 = menu->addMenu(icon, dir->caption());
        addAppMnu(menu2, service->name());
      }
    } else {
      addApp(menu, service->property("DesktopEntryPath").toString());
    }
  }
  return true;
}

void DesktopCustomMenu::switchTo(QAction *action)
{
    QString source = action->data().toString();
    kDebug() << source;
    if (source.endsWith(".desktop")) {
      Plasma::Service *service = dataEngine("apps")->serviceForSource(source);
      if (service) {
          service->startOperationCall(service->operationDescription("launch"));
      }
    } else {
      if (!source.isEmpty()) {
        QStringList cmd = cmd << source.split(" ");;
        KProcess *process = new KProcess(0);
        process->startDetached(cmd);
      }
    }

}

#include "launch.moc"
