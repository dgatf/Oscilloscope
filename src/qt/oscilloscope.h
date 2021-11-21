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

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QPainter>
#include <QQuickImageProvider>
#include <QFile>
#include <QDebug>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

class Oscilloscope : public QObject, public QQuickImageProvider
{
    Q_OBJECT
    //Q_PROPERTY(bool isPaused READ getIsPaused WRITE setIsPaused NOTIFY isPausedChanged)


public:
    enum TriggerType { none, rising, falling };
    enum Status { connected, disconnected, permission_not_granted, error_connecting, error_opening, connection_lost, driver_not_found, permission_requested };
    Q_ENUM(Status)
    Q_ENUM(TriggerType)
#ifdef Q_OS_ANDROID
    Oscilloscope(qreal pixelratio, jobject context);
#else
    Oscilloscope(qreal pixelratio);
#endif
    ~Oscilloscope();
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
    Q_INVOKABLE void openSerialPort(QString portName, QString baudrate, QString dataBits, QString parity, QString interval);
    Q_INVOKABLE void closeSerialPort();
    Q_INVOKABLE QStringList fillPortsInfo();
    Q_INVOKABLE void setBackgroundColor(const QString &color) { backgroundColor = color; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setGridColor(const QString &color) { gridColor = color; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setSignalColor(const QString &color) { signalColor = color; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setTextColor(const QString &color) { textColor = color; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setVDiv(const quint16 value) { vDiv = value; voltRange = value * 6 / 1000.0; yZeroV = getYZeroV(); drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setHDiv(const quint16 value) { hDiv = value; timeRange = value * 11 / 1000.0; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setTriggerType(const quint16 value) { triggerType = (TriggerType)value; }
    Q_INVOKABLE void setTriggerValue(const quint16 value) { triggerValue = value; }
    Q_INVOKABLE void setVAdjust(const qint16 value) { vAdjust = 6/voltRange * value / 100.0; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void setHAdjust(const qint16 value) { hAdjust = 11/timeRange * value / 100.0 * 0.25; drawBackground(); emit sendPixmap(); }
    Q_INVOKABLE void exportImage(QUrl filename);
    Q_INVOKABLE void exportCsv(QUrl filename);
    Q_INVOKABLE bool getIsPaused() { return isPaused; }
    Q_INVOKABLE void setIsPaused(const bool value);
    Q_INVOKABLE QString getVersion() { return APP_VERSION; }
#ifdef Q_OS_ANDROID
    static Oscilloscope *instance() { return m_instance; }
#endif

signals:
    void isPausedChanged(bool isPaused);
    void sendPixmap();
    void sendStatusConn(Status status);
    void sendMessage(QString message, quint16 duration=0);
#ifdef Q_OS_ANDROID
    void receiveBuffer(const QByteArray &buffer);
    void receiveStatus(int status, const QString &msg);
#endif

private:
    enum Edge { min, max };
    quint16 pmHeight = 400; //230;
    quint16 pmWidth = 800; //520;
    quint8 maxValue = 0;
    quint8 minValue = 0;
    quint16 rawFreq = 0;
    quint16 rawDuty = 0;
    bool pendingExport = false;
    bool pendingCsv = false;
    bool pendingPause = false;
    QString filename;
    quint16 interval = 26;
    quint16 vDiv = 1000;
    quint16 hDiv = 1000;
    float voltRange = 1000 * 6 / 1000.0;
    float timeRange = 1000 * 11 / 1000.0;
    qint16 yZeroV;
    quint16 triggerValue = 1000;
    TriggerType triggerType = rising;
    QString backgroundColor;
    QString gridColor;
    QString signalColor;
    QString textColor;
    float vAdjust = 0;
    float hAdjust = 0;
    Status status = disconnected;
    qreal pixelRatio;
    bool isPaused = false;
    bool isCapturing = false;
    QByteArray *captureBuffer;
    QPixmap *pixmap;
    QPainter *paint;
#ifdef Q_OS_ANDROID
    static Oscilloscope *m_instance;
    QJniEnvironment env;
    QJniObject usbSerial;
    jobject context;
    void readStatus(int status, const QString &msg);
#else
    QSerialPort *serial = nullptr;
#endif
    void drawBackground();
    void processData(const QByteArray &data);
    qint16 getYZeroV();
    quint16 getYVal(quint8 value);
    void savePixmap(QString filename);
    void saveCsv(QString filename);

public slots:
#ifndef Q_OS_ANDROID
    void readData();
#else
    void readData(const QByteArray &data);
#endif
};

#endif // OSCILLOSCOPE_H
