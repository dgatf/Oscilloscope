package org.qtproject.qt;

import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import java.io.IOException;
import java.util.List;
import java.util.Arrays;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

public class UsbSerialInterface implements SerialInputOutputManager.Listener
{
    private static native void sendBuffer(byte[] buffer);
    private static native void sendStatus(int status, String msg);

    enum Status { CONNECTED, DISCONNECTED, PERMISSION_NOT_GRANTED, ERROR_CONNECTING, ERROR_OPENING, CONNECTION_LOST };

    UsbManager manager;
    UsbSerialDriver driver;
    UsbSerialPort port;
    UsbDeviceConnection connection;
    SerialInputOutputManager ioManager;
    List<UsbSerialDriver> availableDrivers;
    int baudrate;
    int databits;
    int stopbits;
    int parity;
    Context appContext;
    private static final String ACTION_USB_PERMISSION = "com.example.usbserial.USB_PERMISSION";

    public void sendStatus(int status)
    {
        sendStatus(status, "");
    }

    public void setContext(Context context)
    {
        appContext = context;
    }

    public void closeConnection()
    {
        connection.close();
    }

    public List drivers(Context context)
    {
        manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);
        return availableDrivers;
    }

    public String openConnection(Context context, String name, int baudrate, int databits, int stopbits, int parity)
    {
        this.baudrate = baudrate;
        this.databits = databits;
        this.stopbits = stopbits;
        this.parity = parity;
        driver = null;
        for (int i = 0; i < availableDrivers.size(); i++) {
            if (availableDrivers.get(i).toString().contains(name))
                driver = availableDrivers.get(i);
        }
        if (driver == null)
            return "Driver not found";
        PendingIntent pi = PendingIntent.getBroadcast(context, 0, new Intent(ACTION_USB_PERMISSION), 0);
        context.registerReceiver(receiver, new IntentFilter(ACTION_USB_PERMISSION));
        manager.requestPermission(driver.getDevice(), pi);
        return "Permission requested";
    }

    public void configConnection()
    {

        connection = manager.openDevice(driver.getDevice());
        if (connection == null) {
            sendStatus(Status.ERROR_CONNECTING.ordinal());
            return;
        }

        port = driver.getPorts().get(0);

        try {
            port.open(connection);
            port.setParameters(baudrate,databits,stopbits, parity);
        } catch (IOException e) {
            e.printStackTrace();
            sendStatus(Status.ERROR_OPENING.ordinal());
            return;
        }
        ioManager = new SerialInputOutputManager(port, this);
        ioManager.start();
        sendStatus(Status.CONNECTED.ordinal(), port.toString());
    }

    @Override
    public void onNewData(byte[] data)
    {
        sendBuffer(data);
    }

    @Override
    public void onRunError(Exception e)
    {
        sendStatus(Status.CONNECTION_LOST.ordinal());
    }

    // Receive request Usb permission
    public final BroadcastReceiver receiver = new BroadcastReceiver()
    {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (context) {
                    UsbDevice device = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        if (device != null) {
                            configConnection();
                        }
                    } else {
                            sendStatus(Status.PERMISSION_NOT_GRANTED.ordinal());
                    }
                }
            }
        }
    };
}
