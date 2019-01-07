#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QString>
#include <QFont>
#include <QFontDatabase>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <ctime>
#include <cstdlib>
#include <QTimer>
#include <QQmlContext>
#include <QGeoCoordinate>
#include <QRegExpValidator>
#include <ctime>
#include <QVBoxLayout>

class TrackPoint {
public:
    double x, y;
    int year, month, day, hour, minute;

    TrackPoint() {}
    TrackPoint(double x, double y, int year, int month, int day, int hour, int minute)
    {this->x = x; this->y = y; this->year = year; this->month = month; this->day = day; this->hour = hour; this->minute = minute;}
};

class Track {
public:
    int hunter_id;
    QString name;
    QVector<TrackPoint> track_points;
};

class ChatMessage {
public:
    QString name;
    QString text;
};

class MapCore : public QObject
{
    Q_OBJECT
public:
    explicit MapCore(QObject *parent = nullptr);

signals:
    // Сигнал для передачи данных в qml-интерфейс
    void clearMap();
    void addPolyline();
    void addPoint(QGeoCoordinate position);

public slots:
    // Слот для приёма данных из qml-интерфейса
    //void receiveFromQml();
    void clearMapS() {emit clearMap();}
    void addPolylineS() {emit addPolyline();}
    void addPointS(QGeoCoordinate position) {emit addPoint(position);}

//private:
    //int count;  // Счетчик, которым будем оперировать
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static const unsigned long int SF_SCALEINTERFACE = 0x0001;
    static const unsigned long int SF_SCALEFONT = 0x0002;
    static const unsigned long int SF_SCALESTYLE = 0x0004;
    static const unsigned long int SF_ALL = SF_SCALEINTERFACE | SF_SCALEFONT | SF_SCALESTYLE;

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager *http_manager, *http_manager2;
    MapCore *mapCore;
    float interface_scale = 1.0f;

    QTimer *track_point_timer;
    QTimer *send_track_timer;
    QTimer *update_tracks_timer;

    QString ip_address;

    //data
    bool is_authorized;
    QString token;
    int hunter_id;
    QString name;

    QVector<TrackPoint> send_track_points;
    int ltp_count; // количество отправляемых вершин
    QVector<Track> hunters_tracks;
    QVector<ChatMessage> messages;

    // temp
    double x_coordinate, y_coordinate;

    //if (hunters_tracks.count() > 0)
        //qDebug() << "On get tracks: " << hunters_tracks[0].track_points[1].day;
    // app functions
    bool loadOperation();
    void saveOperation();
    void quiteOperation();

signals:
    void messageSuccesfulSend();
    void messagesSuccesfulGet();

public slots:
    void initFont();
    void initShadows();
    void initValidator();
    void scaleInterface(QWidget *widget, float factor, unsigned long int flags = SF_ALL);
    void resetFont(QWidget *widget);
    void startHunterSession();

    // interface action
    void connectAction();
    void onConnectAction(QNetworkReply*reply);
    void toMainPage();
    void toMapPage();
    void toMessengerPage();
    void toSettingsPage();
    void toInfoPage();
    void quiteAction();

    // трекинг
    void startTracking();
    void endTracking();
    void getTrackPoint();
    void sendTrack();
    void onSendTrack(QNetworkReply *reply);
    void onTrackingButton();

    // получение данных треков
    void getHuntersTracks();
    void onGetHuntersTracks(QNetworkReply *reply);
    void updateMapTracks();

    //чат
    void sendChatMessage();
    void onSendChatMessage(QNetworkReply *reply);
    void getChatMessages();
    void onGetChatMessages(QNetworkReply *reply);
    void updateChatInterface();
};

#endif // MAINWINDOW_H
