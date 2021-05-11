/*
* The MIT License (MIT)
*
* Copyright (c) 2015 Dmitry Ivanov
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include "ColorPickerButton.h"
#include "ColorPickerActionWidget.h"
#include <QColorDialog>
#include <QMenu>

ColorPickerActionWidget::ColorPickerActionWidget(QWidget * parent) :
    QWidgetAction(parent),
    m_colorDialog(new QColorDialog(parent))
{
    m_colorDialog->setWindowFlags(Qt::Widget);
    m_colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);

    QColor currentColor = m_colorDialog->currentColor();
    currentColor.setAlpha(255);
    m_colorDialog->setCurrentColor(currentColor);

    QObject::connect(m_colorDialog, SIGNAL(colorSelected(QColor)), this, SIGNAL(colorSelected(QColor)));
    QObject::connect(m_colorDialog, SIGNAL(rejected()), this, SIGNAL(rejected()));

    setDefaultWidget(m_colorDialog);
}

void ColorPickerActionWidget::aboutToShow() { m_colorDialog->show(); }
void ColorPickerActionWidget::aboutToHide() { m_colorDialog->hide(); }

//----------------------------------------------------------------------

ColorPickerButton::ColorPickerButton(QWidget * parent) :
    QToolButton(parent),
    m_menu(new QMenu(this))
{
    ColorPickerActionWidget * colorPickerActionWidget = new ColorPickerActionWidget(this);
    m_menu->addAction(colorPickerActionWidget);
    setMenu(m_menu);

    QAction * colorDialogAction = new QAction(this);
    setDefaultAction(colorDialogAction);

    QObject::connect(colorDialogAction, SIGNAL(triggered(bool)), this, SLOT(onColorDialogAction()));
    QObject::connect(colorPickerActionWidget, SIGNAL(colorSelected(QColor)), this, SIGNAL(colorSelected(QColor)));
    QObject::connect(colorPickerActionWidget, SIGNAL(rejected()), this, SIGNAL(rejected()));
    QObject::connect(colorPickerActionWidget, SIGNAL(colorSelected(QColor)), m_menu, SLOT(hide()));
    QObject::connect(colorPickerActionWidget, SIGNAL(rejected()), m_menu, SLOT(hide()));
    QObject::connect(m_menu, SIGNAL(aboutToShow()), colorPickerActionWidget, SLOT(aboutToShow()));
    QObject::connect(m_menu, SIGNAL(aboutToHide()), colorPickerActionWidget, SLOT(aboutToHide()));
}

QColor ColorPickerButton::getColor() {
	currentColor = palette().color(QPalette::Button);
	return currentColor;
}

void ColorPickerButton::onColorDialogAction() {
    QScopedPointer<QColorDialog> colorDialogPtr(new QColorDialog(this));
    QColorDialog * colorDialog = colorDialogPtr.data();
    colorDialog->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);

    currentColor = colorDialog->currentColor();
    currentColor.setAlpha(255);
    colorDialog->setCurrentColor(currentColor);

    if (colorDialog->exec() == QColorDialog::Accepted) {
        QColor color = colorDialog->currentColor();
        emit colorSelected(color);
    }
    else emit rejected();
}
