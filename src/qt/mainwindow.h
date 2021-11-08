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
#include <QDebug>
#include <QPainter>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QToolBar>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

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
#ifdef Q_OS_ANDROID
    static MainWindow *instance() { return m_instance; }

signals:
    void receiveBuffer(const QByteArray &buffer);
    void receiveStatus(int status, const QString &msg);
#endif

private:
    enum Edge { min, max };
    enum TriggerType { none, rising, falling };
    Ui::MainWindow *ui;
    QPixmap *m_pixmap;
    QPainter *m_paint;
    SettingsDialog *m_ui_settings = nullptr;

    uint16_t interval = 26; // us
    float vDiv = 1; // v
    float volt = 1 * 6;
    float hDiv = 1; // ms
    uint16_t timeLenght = hDiv * 10;
    float trigger = 1;
    TriggerType triggerType = rising;
    uint8_t maxValue = 0;
    uint8_t minValue = 0;
    uint16_t rawFreq = 0;
    uint16_t rawDuty = 0;
    bool pendingExport = false;
    QString fileName;
    QLabel *m_statusLabel = nullptr;
    QAction *connectAct;
    QAction *disconnectAct;

#ifdef Q_OS_ANDROID
    enum Status { CONNECTED, DISCONNECTED, PERMISSION_NOT_GRANTED, ERROR_CONNECTING, ERROR_OPENING, CONNECTION_LOST };
    Status status = Status::DISCONNECTED;
    static MainWindow *m_instance;
    QJniEnvironment env;
    QJniObject usbSerial;
    void readData(const QByteArray &data);
    void readStatus(int status, const QString &msg);
#else
    QSerialPort *m_serial = nullptr;
#endif
    void drawBackground();
    void processData(const QByteArray &data);
    void setJni();

private slots:
    void openSerialPort();
    void closeSerialPort();
    void updateTrigger();
    void updateTriggerType();
    void exitApp();
    void updateVdiv();
    void updateHdiv();
    void settings();
    void about();
    void exportData();
    void refresh();
#ifndef Q_OS_ANDROID
    void readData();
#else
    //void readData(const QByteArray &data);
#endif
};

#endif // MAINWINDOW_H
