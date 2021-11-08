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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QDebug>
#include <QLineEdit>
#include <QColorDialog>
#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

QT_BEGIN_NAMESPACE

namespace Ui
{
class SettingsDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        qint16 interval;
        QString backgroundcolor;
        QString gridcolor;
        QString signalcolor;
        QString textcolor;
    };

    enum Element {background, grid, signal, text};
    Settings m_currentSettings;
    QSettings *m_settings;
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    Settings settings() const;
#ifdef Q_OS_ANDROID
    void fillPortsInfo(QJniObject &usbSerial);
#endif

private slots:
    void apply();
    void checkCustomBaudRatePolicy(int idx);
    void checkCustomDevicePathPolicy(int idx);
    void color(Element element);

private:
    Ui::SettingsDialog *m_ui = nullptr;
    QIntValidator *m_intValidator = nullptr;
    QString backgroundcolor;
    QString gridcolor;
    QString signalcolor;
    QString textcolor;
#ifndef Q_OS_ANDROID
    void fillPortsInfo();
#endif
    void fillPortsParameters();
    void updateSettings();
};

#endif // SETTINGSDIALOG_H
