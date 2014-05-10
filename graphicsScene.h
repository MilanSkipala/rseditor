/*
    Slot track and model railway editor by Milan Skipala
    Copyright (C) 2014 Milan Skipala

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include "includeHeaders.h"

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT
    //bool mouseMode; //F=select activeEP by doubleClick, T=create aEP by doubleClick
    QMenu * contextMenuItem;
    QMenu * contextMenuScene;

public:
    GraphicsScene(QMenu * contextMenuItem, QMenu * contextMenuScene, qreal x, qreal y, qreal width, qreal height, QObject *parent = 0);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

public slots:
    //void toggleMode();

};


#endif //GRAPHICSSCENE_H
