// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
code based on source from Gwenview
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Qt
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

// Local
#include "pimcommon/widgets/slidecontainer.h"

using namespace PimCommon;

class Window : public QWidget
{
public:
    Window()
        : QWidget()
        {
        SlideContainer* container = new SlideContainer(this);

        QPushButton* inButton = new QPushButton(this);
        inButton->setText(QLatin1String("Slide &In"));
        connect(inButton, SIGNAL(clicked()), container, SLOT(slideIn()));

        QPushButton* outButton = new QPushButton(this);
        outButton->setText(QLatin1String("Slide &Out"));
        connect(outButton, SIGNAL(clicked()), container, SLOT(slideOut()));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(inButton);
        layout->addWidget(outButton);
        layout->addWidget(container);

        QLineEdit* content = new QLineEdit(container);
        content->setText(QLatin1String("Some long text. Some long text. Some long text. Some long text."));
        container->setContent(content);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    Window window;

    window.show();
    return app.exec();
}

