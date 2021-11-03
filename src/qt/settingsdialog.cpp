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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

SettingsDialog::SettingsDialog(QWidget* parent) :
    QDialog(parent),
    m_settings(new QSettings("Oscilloscope", "DGeA Soft")),
    m_ui(new Ui::SettingsDialog),
    m_intValidator(new QIntValidator(0, 4000000, this))

{
    m_ui->setupUi(this);
    m_ui->baudRateBox->setInsertPolicy(QComboBox::NoInsert);
    connect(m_ui->applyButton, &QPushButton::clicked, this, &SettingsDialog::apply);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::showPortInfo);
    connect(m_ui->baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::checkCustomBaudRatePolicy);
    connect(m_ui->serialPortInfoListBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsDialog::checkCustomDevicePathPolicy);
    connect(m_ui->btBackground, &QPushButton::clicked, this, [this] {color(Element::background);});
    connect(m_ui->btGrid, &QPushButton::clicked, this, [this] {color(Element::grid);});
    connect(m_ui->btSignal, &QPushButton::clicked, this, [this] {color(Element::signal);});
    connect(m_ui->btText, &QPushButton::clicked, this, [this] {color(Element::text);});
    fillPortsParameters();
    fillPortsInfo();
    m_ui->lbBackground->setStyleSheet("QLabel { background-color : #000000;}");
    m_ui->lbGrid->setStyleSheet("QLabel { background-color : #33d17a;}");
    m_ui->lbSignal->setStyleSheet("QLabel { background-color : #33d17a;}");
    m_ui->lbText->setStyleSheet("QLabel { background-color : #c061cb;}");
    backgroundcolor = "#000000";
    gridcolor = "#33d17a";
    signalcolor = "#33d17a";
    textcolor = "#9141ac";

    if (m_settings->value("serialport").isValid()) {
        m_ui->serialPortInfoListBox->setCurrentText(m_settings->value("serialport").toString());
    }

    if (m_settings->value("baudrate").isValid()) {
        m_ui->baudRateBox->setCurrentText(m_settings->value("baudrate").toString());
    }

    if (m_settings->value("databits").isValid()) {
        m_ui->dataBitsBox->setCurrentText(m_settings->value("databits").toString());
    }

    if (m_settings->value("parity").isValid()) {
        m_ui->parityBox->setCurrentText(m_settings->value("parity").toString());
    }

    if (m_settings->value("interval").isValid()) {
        m_ui->sbInterval->setValue(m_settings->value("interval").toInt());
    }

    backgroundcolor = m_settings->value("backgroundcolor").toString();
    gridcolor = m_settings->value("gridcolor").toString();
    signalcolor = m_settings->value("signalcolor").toString();
    textcolor = m_settings->value("textcolor").toString();
    if (m_settings->value("backgroundcolor").isValid()) {
        m_ui->lbBackground->setStyleSheet("QLabel { background-color : " + backgroundcolor + ";}");
    }

    if (m_settings->value("gridcolor").isValid()) {
        m_ui->lbGrid->setStyleSheet("QLabel { background-color : " + gridcolor + ";}");
    }

    if (m_settings->value("signalcolor").isValid()) {
        m_ui->lbSignal->setStyleSheet("QLabel { background-color : " + signalcolor + ";}");
    }

    if (m_settings->value("textcolor").isValid()) {
        m_ui->lbText->setStyleSheet("QLabel { background-color : " + textcolor + ";}");
    }

    updateSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

SettingsDialog::Settings SettingsDialog::settings() const
{
    return m_currentSettings;
}

void SettingsDialog::showPortInfo(int idx)
{
    if (idx == -1) {
        return;
    }

    const QStringList list = m_ui->serialPortInfoListBox->itemData(idx).toStringList();
}

void SettingsDialog::apply()
{
    updateSettings();
    accept();
}

void SettingsDialog::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !m_ui->baudRateBox->itemData(idx).isValid();
    m_ui->baudRateBox->setEditable(isCustomBaudRate);

    if (isCustomBaudRate) {
        m_ui->baudRateBox->clearEditText();
        QLineEdit* edit = m_ui->baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}

void SettingsDialog::checkCustomDevicePathPolicy(int idx)
{
    const bool isCustomPath = !m_ui->serialPortInfoListBox->itemData(idx).isValid();
    m_ui->serialPortInfoListBox->setEditable(isCustomPath);

    if (isCustomPath) {
        m_ui->serialPortInfoListBox->clearEditText();
    }
}

void SettingsDialog::fillPortsParameters()
{
    m_ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    m_ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    m_ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    m_ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    m_ui->baudRateBox->addItem(QStringLiteral("230400"), 230400);
    m_ui->baudRateBox->addItem(QStringLiteral("500000"), 500000);
    m_ui->baudRateBox->addItem(QStringLiteral("1000000"), 1000000);
    m_ui->baudRateBox->setCurrentIndex(6);
    m_ui->baudRateBox->addItem(tr("Custom"));
    m_ui->dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    m_ui->dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    m_ui->dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    m_ui->dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    m_ui->dataBitsBox->setCurrentIndex(3);
    m_ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    m_ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    m_ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    m_ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    m_ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity); 
}

void SettingsDialog::fillPortsInfo()
{
    m_ui->serialPortInfoListBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo& info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
        m_ui->serialPortInfoListBox->addItem(list.first(), list);
    }

    m_ui->serialPortInfoListBox->addItem(tr("Custom"));
}

void SettingsDialog::updateSettings()
{
    m_currentSettings.name = m_ui->serialPortInfoListBox->currentText();
    m_settings->setValue("serialport", m_ui->serialPortInfoListBox->currentText());

    if (m_ui->baudRateBox->currentIndex() == 4) {
        m_currentSettings.baudRate = m_ui->baudRateBox->currentText().toInt();
    } else {
        m_currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(
                                         m_ui->baudRateBox->itemData(m_ui->baudRateBox->currentIndex()).toInt());
    }

    m_currentSettings.stringBaudRate = QString::number(m_currentSettings.baudRate);
    m_settings->setValue("baudrate", m_ui->baudRateBox->currentText().toInt());
    m_currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                                     m_ui->dataBitsBox->itemData(m_ui->dataBitsBox->currentIndex()).toInt());
    m_currentSettings.stringDataBits = m_ui->dataBitsBox->currentText();
    m_settings->setValue("databits", m_ui->dataBitsBox->currentText().toInt());
    m_currentSettings.parity = static_cast<QSerialPort::Parity>(
                                   m_ui->parityBox->itemData(m_ui->parityBox->currentIndex()).toInt());
    m_currentSettings.stringParity = m_ui->parityBox->currentText();
    m_settings->setValue("parity", m_ui->parityBox->currentIndex());

    m_currentSettings.interval = m_ui->sbInterval->value();
    m_settings->setValue("interval", m_ui->sbInterval->value());

    m_currentSettings.backgroundcolor = backgroundcolor;
    m_settings->setValue("backgroundcolor", backgroundcolor);
    m_currentSettings.gridcolor = gridcolor;
    m_settings->setValue("gridcolor", gridcolor);
    m_currentSettings.signalcolor = signalcolor;
    m_settings->setValue("signalcolor", signalcolor);
    m_currentSettings.textcolor = textcolor;
    m_settings->setValue("textcolor", textcolor);
}

void SettingsDialog::color(Element element)
{
    QColor color = QColorDialog::getColor(Qt::yellow, this);

    qInfo() << color.name();
    if (color.isValid()) {
        if (element == Element::background) {
            backgroundcolor = color.name();
            m_ui->lbBackground->setStyleSheet("QLabel { background-color : " + backgroundcolor + ";}");
        } else if (element == Element::grid) {
            gridcolor = color.name();
            m_ui->lbGrid->setStyleSheet("QLabel { background-color : " + gridcolor + ";}");
        } else if (element == Element::signal) {
            signalcolor = color.name();
            m_ui->lbSignal->setStyleSheet("QLabel { background-color : " + signalcolor + ";}");
        } else if (element == Element::text) {
            textcolor = color.name();
            m_ui->lbText->setStyleSheet("QLabel { background-color : " + textcolor + ";}");
        }
    }
}
