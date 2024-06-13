/*
 * Copyright 2024, Tim Baker <treectrl@users.sf.net>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "pngzonesdialog.h"
#include "ui_pngzonesdialog.h"

#include "world.h"
#include "worldcell.h"

#include "InGameMap/clipper.hpp"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>

PNGZonesDialog::PNGZonesDialog(World *world, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PNGZonesDialog),
    mWorld(world),
    mPainter(nullptr)
{
    ui->setupUi(this);

    connect(ui->pngBrowse, &QAbstractButton::clicked, this, &PNGZonesDialog::browse);
}

PNGZonesDialog::~PNGZonesDialog()
{
    delete ui;
}

bool PNGZonesDialog::generateWorld(World *world)
{
    mImage = QImage(world->size() * 300, QImage::Format_RGBA8888);
    mImage.fill(Qt::lightGray);

    QPainter painter(&mImage);
    mPainter = &painter;

    for (int y = 0; y < world->height(); y++) {
        for (int x = 0; x < world->width(); x++) {
            if (!generateCell(world->cellAt(x, y)))
                goto errorExit;
        }
    }

    painter.end();

    if (!mImage.save(ui->pngEdit->text()))
        goto errorExit;

    return true;

errorExit:
    return false;
}

static QPolygonF createPolylineOutline(WorldCellObject *object)
{
    ClipperLib::ClipperOffset offset;
    ClipperLib::Path path;
    int SCALE = 100;
    for (int i = 0; i < object->points().size(); i++) {
        WorldCellObjectPoint p1 = object->points()[i];
        path << ClipperLib::IntPoint(p1.x * SCALE, p1.y * SCALE);
        if ((object->polylineWidth() % 2) != 0) {
            ClipperLib::IntPoint cp = path[path.size()-1];
            path[path.size()-1] = ClipperLib::IntPoint(cp.X + SCALE / 2, cp.Y + SCALE / 2);
        }
    }
    offset.AddPath(path, ClipperLib::JoinType::jtMiter, ClipperLib::EndType::etOpenButt);
    ClipperLib::Paths paths;
    offset.Execute(paths, object->polylineWidth() * SCALE / 2.0);
    QPolygonF result;
    if (paths.empty()) {
        return result;
    }
    int cellX = object->cell()->x();
    int cellY = object->cell()->y();
    ClipperLib::Path cPath = paths.at(0);
    for (const auto &cPoint : cPath) {
        result << QPointF(cellX * 300 + cPoint.X / (qreal) SCALE, cellY * 300.0 + cPoint.Y / (qreal) SCALE);
    }
    return result;
}

bool PNGZonesDialog::generateCell(WorldCell *cell)
{
    QPainter *painter = mPainter;
    QStringList validZones;
    validZones << QLatin1String("DeepForest");
    validZones << QLatin1String("Farm");
    validZones << QLatin1String("FarmLand");
    validZones << QLatin1String("Forest");
    validZones << QLatin1String("Nav");
    validZones << QLatin1String("TownZone");
    validZones << QLatin1String("TrailerPark");
    validZones << QLatin1String("Vegitation");
    for (WorldCellObject *object : cell->objects()) {
        if (object->group() == nullptr) {
            continue;
        }
        if (validZones.contains(object->group()->name()) == false)
            continue;
        QColor color = object->group()->color();
//        color.setAlpha(50);
        painter->setBrush(QBrush(color));
        painter->setPen(Qt::NoPen);
        if (object->isRectangle()) {
            QPointF p1(cell->x() * 300.0 + object->x(), cell->y() * 300.0 + object->y());
            QPointF p2(cell->x() * 300.0 + (object->x() + object->width()), cell->y() * 300.0 + (object->y() + object->height()));
            painter->drawRect(QRectF(p1, p2));
        }
        if (object->isPolyline()) {
            painter->drawPolygon(createPolylineOutline(object));
        }
        if (object->isPolygon()) {
            QPolygonF poly;
            for (WorldCellObjectPoint pt : object->points()) {
                poly << QPointF(cell->x() * 300.0 + pt.x, cell->y() * 300.0 + pt.y);
            }
            painter->drawPolygon(poly);
        }
#if 0
        if (object->isPolyline() && (object->polylineWidth() > 0)) {
            if (mPolylineOutlines.contains(object) == false) {
                mPolylineOutlines[object] = createPolylineOutline(mScene, object);
            }
            QPolygonF poly = mPolylineOutlines[object];
            painter->drawPolygon(poly);
        }
#endif
    }
    return true;
}

void PNGZonesDialog::accept()
{
    if (ui->pngEdit->text().isEmpty())
        return;

    if (!generateWorld(mWorld)) {
        QMessageBox::warning(this, tr("PNG Generation Failed"), mError);
    }

    QDialog::accept();
}

void PNGZonesDialog::browse()
{
    QString f = QFileDialog::getSaveFileName(this, tr("Save PNG As..."),
                                             ui->pngEdit->text(), QLatin1String("PNG Files (*.png)"));
    if (f.isEmpty())
        return;
    ui->pngEdit->setText(QDir::toNativeSeparators(f));
}
