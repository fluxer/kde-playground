/*
Copyright 2007 Aurélien Gâteau

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef SVGPART_H
#define SVGPART_H

// KDE
#include <kparts/part.h>


#include <QGraphicsScene>
#include <QGraphicsSvgItem>
#include <QGraphicsView>
#include <QSvgRenderer>

class SvgPart : public KParts::ReadOnlyPart {
	Q_OBJECT
public:
	SvgPart(QWidget* parentWidget, QObject* parent, const QVariantList&);

	virtual bool closeUrl();

protected:
	virtual bool openFile();

private Q_SLOTS:
	void zoomActualSize();
	void zoomIn();
	void zoomOut();

private:
	QGraphicsScene* mScene;
	QGraphicsView* mView;
	QGraphicsSvgItem* mItem;
	QSvgRenderer* mRenderer;

	qreal zoom() const;
	void setZoom(qreal);
};

#endif /* SVGPART_H */
