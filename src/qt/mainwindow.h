/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QToolBar>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class SettingsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void resizeEvent(QResizeEvent* event);

    uint16_t interval; // us

private:
    Ui::MainWindow *ui;
    QSerialPort *m_serial = nullptr;
    QPixmap *m_pixmap;
    QPainter *m_paint;
    SettingsDialog *m_ui_settings = nullptr;
    QLabel *m_statusLabel = nullptr;

    float vDiv = 1; // v
    float volt = 1 * 6;
    float hDiv = 1; // ms
    uint16_t timeLenght = hDiv * 10;
    float trigger = 1;
    enum Edge { min, max };
    enum TriggerType { none, rising, falling };
    TriggerType triggerType = none;

    uint8_t maxValue = 0;
    uint8_t minValue = 0;
    uint16_t rawFreq;
    uint16_t rawDuty;
    bool pendingExport = false;
    QString fileName;

    QAction *connectAct;
    QAction *disconnectAct;

    void drawBackground();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void readData();
    void updateTrigger();
    void updateTriggerType();
    void exitApp();
    void updateVdiv();
    void updateHdiv();
    void settings();
    void about();
    void exportData();
    void refresh();
};

#endif // MAINWINDOW_H
