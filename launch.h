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

#ifndef DESKTOPCUSTOMMENU_HEADER
#define DESKTOPCUSTOMMENU_HEADER

#include "ui_config.h"
#include <plasma/containmentactions.h>

class QAction;
class KMenu;

class DesktopCustomMenu : public Plasma::ContainmentActions
{
    Q_OBJECT
    public:
        DesktopCustomMenu(QObject* parent, const QVariantList& args);
        ~DesktopCustomMenu();

        void init(const KConfigGroup &config);
        QWidget* createConfigurationInterface(QWidget* parent);
        void configurationAccepted();
        void save(KConfigGroup &config);

        void contextEvent(QEvent *event);
        QList<QAction *> contextualActions();
        
        bool addCmd(QMenu *menu, const QString &txt, const QString &iconpath, const QString &cmd);
        QMenu* addMnu(QMenu *menu, const QString &txt, const QString &iconpath);
        bool addApp(QMenu *menu, const QString &source);
        bool addAppMnu(QMenu *menu, const QString &path);

    public slots:
        void switchTo(QAction *action);

    protected:
        void makeMenu();

    private:
        KMenu *m_menu;
        QAction *m_action;
        Ui::Config ui;
        QString menuconfig;
};

K_EXPORT_PLASMA_CONTAINMENTACTIONS(desktopcustommenu, DesktopCustomMenu)

#endif
