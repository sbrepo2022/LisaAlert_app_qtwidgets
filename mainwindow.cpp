#include "mainwindow.h"
#include "ui_mainwindow.h"

// app functions
bool MainWindow::loadOperation() {
    QFile file("operation_data.txt");
    QString data;
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        data = file.readAll();
        token = data.section('|', 0, 0);
        hunter_id = data.section('|', 1, 1).toInt();
        name = data.section('|', 2, 2);
        return true;
    }
    else {
        return false;
    }
}

void MainWindow::saveOperation() {
    QFile file("operation_data.txt");
    if (file.exists()) {
        return;
    }
    else {
        file.open(QIODevice::WriteOnly);
        file.write(token.toUtf8());
        file.write("|");
        file.write(QString::number(hunter_id).toUtf8());
        file.write("|");
        file.write(name.toUtf8());
        return;
    }
}

void MainWindow::quiteOperation() {
    QFile file("operation_data.txt");
    if (file.exists())
        file.remove();
}

// map core
MapCore::MapCore(QObject *parent) {

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ip_address = "fork.pythonanywhere.com";
    ltp_count = 0;
    is_authorized = false;

    // network init
    http_manager = new QNetworkAccessManager;
    http_manager2 = new QNetworkAccessManager;

    // connections
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectAction()));
    connect(ui->mapMenuButton, SIGNAL(clicked()), this, SLOT(toMapPage()));
    connect(ui->messengerMenuButton, SIGNAL(clicked()), this, SLOT(toMessengerPage()));
    connect(ui->infoMenuButton, SIGNAL(clicked()), this, SLOT(toInfoPage()));
    connect(ui->settingsMenuButton, SIGNAL(clicked()), this, SLOT(toSettingsPage()));
    connect(ui->trackingButton, SIGNAL(clicked()), this, SLOT(onTrackingButton()));
    connect(ui->quiteButton, SIGNAL(clicked()), this, SLOT(quiteAction()));
    connect(ui->sendMessageButton, SIGNAL(clicked()), this, SLOT(sendChatMessage()));

    connect(this, SIGNAL(messageSuccesfulSend()), this, SLOT(getChatMessages()));
    connect(this, SIGNAL(messagesSuccesfulGet()), this, SLOT(updateChatInterface()));

    initShadows();

    // загрузка шрифтов
    int font_id;
    QString family;

    font_id = QFontDatabase::addApplicationFont(":/fonts/SF-UI-Display-Regular.ttf");
    family = QFontDatabase::applicationFontFamilies(font_id).at(0);

    font_id = QFontDatabase::addApplicationFont(":/fonts/SF-UI-Text-Regular.ttf"); //путь к шрифту
    family = QFontDatabase::applicationFontFamilies(font_id).at(0); //имя шрифта

    font_id = QFontDatabase::addApplicationFont(":/fonts/SF-UI-Text-Bold.ttf"); //путь к шрифту
    family = QFontDatabase::applicationFontFamilies(font_id).at(0); //имя шрифта

    // установка шрифтов
    QTimer::singleShot(0, this, SLOT(initFont())); //инициализация шрифтов по таймеру

    QTimer::singleShot(0, this, SLOT(initValidator())); // инициализация валидатора

    // сброс шрифта
    resetFont(ui->centralWidget);

    // маштабирование интерфейса
     this->setGeometry(this->geometry().x(), this->geometry().y(), this->geometry().width() * interface_scale, this->geometry().height() * interface_scale);
     scaleInterface(ui->centralWidget, interface_scale, SF_ALL);

     // map core
     mapCore = new MapCore();
     QQmlContext *context = ui->mapQuickWidget->rootContext();
     context->setContextProperty("mapCore", mapCore);
     ui->mapQuickWidget->setSource(QUrl::fromLocalFile(":/map.qml"));

     ui->errorLabel->hide();

     track_point_timer = new QTimer();
     connect(track_point_timer, SIGNAL(timeout()), this, SLOT(getTrackPoint()));
     send_track_timer = new QTimer();
     connect(send_track_timer, SIGNAL(timeout()), this, SLOT(sendTrack()));
     update_tracks_timer = new QTimer();
     connect(update_tracks_timer, SIGNAL(timeout()), this, SLOT(getHuntersTracks()));

     // temp
     x_coordinate = 37.586859;
     y_coordinate = 55.778274;
     srand(time(nullptr));

     // в самом конце инициализации
     if (loadOperation())
         startHunterSession();
}

MainWindow::~MainWindow()
{
    if (is_authorized)
        saveOperation();

    delete ui;
}

void MainWindow::initFont() {
    QFont font;

    font = ui->titleLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    font.setBold(true);
    ui->titleLabel->setFont(font);

    font = ui->settingsLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    font.setBold(true);
    ui->settingsLabel->setFont(font);

    font = ui->infoLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    font.setBold(true);
    ui->infoLabel->setFont(font);

    font = ui->loginLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->loginLabel->setFont(font);

    font = ui->layersLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->layersLabel->setFont(font);

    font = ui->errorLabel->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->errorLabel->setFont(font);

    font = ui->tokenEdit->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->tokenEdit->setFont(font);

    font = ui->nameEdit->font();
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->nameEdit->setFont(font);

    font = ui->connectButton->font();
    font.setFamily(QStringLiteral("SF UI Display"));
    ui->connectButton->setFont(font);

    font = ui->quiteButton->font();
    font.setFamily(QStringLiteral("SF UI Display"));
    ui->quiteButton->setFont(font);

    // шрифт виджета
    font.setFamily(QStringLiteral("SF UI Text"));
    ui->nameEdit->setFont(font);
}

void MainWindow::initShadows() {
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius((int)(20 * interface_scale));
    shadow->setOffset(0);
    shadow->setYOffset((int)(5 * interface_scale));
    ui->loginWidget->setGraphicsEffect(shadow);

    QGraphicsDropShadowEffect *shadow2 = new QGraphicsDropShadowEffect(this);
    shadow2->setBlurRadius((int)(20 * interface_scale));
    shadow2->setOffset(0);
    ui->tabWidget->setGraphicsEffect(shadow2);

    QGraphicsDropShadowEffect *shadow3 = new QGraphicsDropShadowEffect(this);
    shadow3->setBlurRadius((int)(10 * interface_scale));
    shadow3->setOffset(0);
    shadow3->setYOffset((int)(5 * interface_scale));
    ui->trackingButton->setGraphicsEffect(shadow3);

    QGraphicsDropShadowEffect *shadow4 = new QGraphicsDropShadowEffect(this);
    shadow4->setBlurRadius((int)(5 * interface_scale));
    shadow4->setOffset(0);
    shadow4->setYOffset((int)(3 * interface_scale));
    ui->tracksLayerButton->setGraphicsEffect(shadow4);

    QGraphicsDropShadowEffect *shadow5 = new QGraphicsDropShadowEffect(this);
    shadow5->setBlurRadius((int)(5 * interface_scale));
    shadow5->setOffset(0);
    shadow5->setYOffset((int)(3 * interface_scale));
    ui->searchAreasLayerButton->setGraphicsEffect(shadow5);

    QGraphicsDropShadowEffect *shadow6 = new QGraphicsDropShadowEffect(this);
    shadow6->setBlurRadius((int)(5 * interface_scale));
    shadow6->setOffset(0);
    shadow6->setYOffset((int)(3 * interface_scale));
    ui->markersLayerButton->setGraphicsEffect(shadow6);

    QGraphicsDropShadowEffect *shadow7 = new QGraphicsDropShadowEffect(this);
    shadow7->setBlurRadius((int)(20 * interface_scale));
    shadow7->setOffset(0);
    shadow7->setYOffset((int)(5 * interface_scale));
    ui->layersWidget->setGraphicsEffect(shadow7);

    QGraphicsDropShadowEffect *shadow71 = new QGraphicsDropShadowEffect(this);
    shadow71->setBlurRadius((int)(20 * interface_scale));
    shadow71->setOffset(0);
    shadow71->setYOffset((int)(5 * interface_scale));
    ui->infoWidget->setGraphicsEffect(shadow71);

    QGraphicsDropShadowEffect *shadow72 = new QGraphicsDropShadowEffect(this);
    shadow72->setBlurRadius((int)(20 * interface_scale));
    shadow72->setOffset(0);
    shadow72->setYOffset((int)(5 * interface_scale));
    ui->messagesScrollArea->setGraphicsEffect(shadow72);

    QGraphicsDropShadowEffect *shadow8 = new QGraphicsDropShadowEffect(this);
    shadow8->setBlurRadius((int)(10 * interface_scale));
    shadow8->setOffset(0);
    shadow8->setYOffset((int)(5 * interface_scale));
    ui->quiteButton->setGraphicsEffect(shadow8);

    QGraphicsDropShadowEffect *shadow81 = new QGraphicsDropShadowEffect(this);
    shadow81->setBlurRadius((int)(10 * interface_scale));
    shadow81->setOffset(0);
    shadow81->setYOffset((int)(5 * interface_scale));
    ui->messageEdit->setGraphicsEffect(shadow81);

    QGraphicsDropShadowEffect *shadow82 = new QGraphicsDropShadowEffect(this);
    shadow82->setBlurRadius((int)(10 * interface_scale));
    shadow82->setOffset(0);
    shadow82->setYOffset((int)(5 * interface_scale));
    ui->sendMessageButton->setGraphicsEffect(shadow82);
}

void MainWindow::initValidator() {
    QRegExpValidator *validator;
    validator = new QRegExpValidator(QRegExp("\\w+"));
    ui->nameEdit->setValidator(validator);
    ui->tokenEdit->setValidator(validator);
}

void MainWindow::scaleInterface(QWidget *widget, float factor, unsigned long flags) {
    QFont font;
    QMargins new_margins;

    if (widget != nullptr && qobject_cast<QWidget*>(widget)) { //qobject_cast<QVBoxLayout*>(widget) != nullptr
        //qDebug() << widget->property("objectName").toString();

        if (flags & SF_SCALEINTERFACE) {
            if (widget->maximumWidth() * factor < 16777215)
                widget->setMaximumWidth(widget->maximumWidth() * factor);

            if (widget->maximumHeight() * factor < 16777215)
                widget->setMaximumHeight(widget->maximumHeight() * factor);

            if (widget->minimumWidth() * factor < 16777215)
                widget->setMinimumWidth(widget->minimumWidth() * factor);

            if (widget->minimumHeight() * factor < 16777215)
                widget->setMinimumHeight(widget->minimumHeight() * factor);

            // content margins
            if (widget->contentsMargins().left() * factor < 16777215)
                new_margins.setLeft(widget->contentsMargins().left() * factor);

            if (widget->contentsMargins().right() * factor < 16777215)
                new_margins.setRight(widget->contentsMargins().right() * factor);

            if (widget->contentsMargins().top() * factor < 16777215)
                new_margins.setTop(widget->contentsMargins().top() * factor);

            if (widget->contentsMargins().bottom() * factor < 16777215)
                new_margins.setBottom(widget->contentsMargins().bottom() * factor);

            widget->setContentsMargins(new_margins);
        }

        if (flags & SF_SCALEFONT) {
            font = widget->font();
            font.setPointSizeF(font.pointSizeF() * factor);
            widget->setFont(font);
        }

        /*if (flags & SF_SCALESTYLE) {
            widget->
        }*/
    }

    QObjectList children_objects = widget->children();
    for (int i = 0; i < children_objects.size(); i++) {
        scaleInterface(reinterpret_cast<QWidget*>(children_objects[i]), factor, flags);
    }
}

void MainWindow::resetFont(QWidget *widget) {
    QFont font;

    if (widget != nullptr && qobject_cast<QWidget*>(widget)) {
        font = widget->font();
        font.setPointSizeF(font.pointSizeF());
        widget->setFont(font);
    }

    QObjectList children_objects = widget->children();
    for (int i = 0; i < children_objects.size(); i++) {
        resetFont(reinterpret_cast<QWidget*>(children_objects[i]));
    }
}

void MainWindow::startHunterSession() {
    toMainPage();
    //startTracking();
    update_tracks_timer->start(5000);
    is_authorized = true;

    ui->tokenViewLabel->setText(token);
    ui->uidViewLabel->setText(QString::number(hunter_id));
    ui->nameViewLabel->setText(name);
}

//interface actions
void MainWindow::connectAction() {
    // проверка токена и имени
    bool is_correct = true;
    QString err_message;
    if (ui->tokenEdit->text().length() < 5) {
        err_message += "Некорректный токен. ";
        ui->tokenEdit->setStyleSheet("background-color: #f8d8d8; border-color: #c55;");
        is_correct = false;
    }
    else {
        ui->tokenEdit->setStyleSheet("background-color: #f8f8f8; border-color: #ccc;");
    }
    if (ui->nameEdit->text().length() == 0) {
        err_message += "Некорректное имя. ";
        ui->nameEdit->setStyleSheet("background-color: #f8d8d8; border-color: #c55;");
        is_correct = false;
    }
    else {
        ui->nameEdit->setStyleSheet("background-color: #f8f8f8; border-color: #ccc;");
    }
    if (! is_correct) {
        ui->errorLabel->setText(err_message);
        ui->errorLabel->show();
    }
    else {
        token = ui->tokenEdit->text();
        name = ui->nameEdit->text();
        ui->errorLabel->setText("Подключение...");
        ui->errorLabel->setStyleSheet("color: #0f0;");
        ui->errorLabel->show();

        connect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onConnectAction(QNetworkReply*)));
        QString url = "http://" + ip_address + "/operations/join_operation/?token=" + token + "&name=" + name;
        //qDebug() << url;
        QNetworkRequest request = QNetworkRequest(QUrl(url));
        http_manager->get(request);
    }
}

void MainWindow::onConnectAction(QNetworkReply *reply) {
    if(reply->error()){
        ui->errorLabel->setText("Ошибка подключения.");
        ui->errorLabel->setStyleSheet("color: #f00;");
        ui->errorLabel->show();
    }
    else {
        QString result = reply->readAll();
        qDebug() << "On connect action: " << result;
        if (result == "error404") {
            ui->errorLabel->setText("Токен не найден.");
            ui->errorLabel->setStyleSheet("color: #f00;");
            ui->errorLabel->show();
        }
        else {
            hunter_id = result.toInt();
            startHunterSession();

            ui->errorLabel->hide();
        }
    }
    disconnect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onConnectAction(QNetworkReply*)));
    reply->deleteLater();
}

void MainWindow::toMainPage() {
    ui->mainStackedWidget->setCurrentIndex(1);
}

void MainWindow::toMapPage() {
    ui->contentStackedWidget->setCurrentIndex(0);
}

void MainWindow::toMessengerPage() {
    ui->contentStackedWidget->setCurrentIndex(1);
}

void MainWindow::toSettingsPage() {
    ui->contentStackedWidget->setCurrentIndex(2);
}

void MainWindow::toInfoPage() {
    ui->contentStackedWidget->setCurrentIndex(3);
}

void MainWindow::quiteAction() {
    quiteOperation();
    endTracking();
    update_tracks_timer->stop();
    is_authorized = false;
    ui->mainStackedWidget->setCurrentIndex(0);

    qDebug() << "qute!";
}

void MainWindow::startTracking() {
    track_point_timer->start(1000);
    send_track_timer->start(6000);
}

void MainWindow::endTracking() {
    track_point_timer->stop();
    send_track_timer->stop();
}

void MainWindow::getTrackPoint() { // работа со временем и геодатой
    send_track_points.push_back(TrackPoint(y_coordinate, x_coordinate, 2018, 11, 23, 12, 15));
    x_coordinate += 0.0003;
    y_coordinate += (float)(rand() % 10 - 5) / 33000.0f;
}

void MainWindow::sendTrack() {
    QString send_data;
    send_data += "{\"tk\":\"";
    send_data += token;
    send_data += "\",\"id\":";
    send_data += QString::number(hunter_id);
    send_data += ",\"g\":[";
    for (int i = 0; i < send_track_points.count(); i++) {
        send_data += "{\"x\":";
        send_data += QString::number(send_track_points[i].x);
        send_data += ",\"y\":";
        send_data += QString::number(send_track_points[i].y);
        send_data += ",\"t\":[";
        send_data += QString::number(send_track_points[i].year);
        send_data += ",";
        send_data += QString::number(send_track_points[i].month);
        send_data += ",";
        send_data += QString::number(send_track_points[i].day);
        send_data += ",";
        send_data += QString::number(send_track_points[i].hour);
        send_data += ",";
        send_data += QString::number(send_track_points[i].minute);
        send_data += "]}";
        if (i < send_track_points.count() - 1) {
            send_data += ",";
        }
    }
    send_data += "]}";

    ltp_count = send_track_points.count();

    connect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSendTrack(QNetworkReply*)));
    QString url = "http://" + ip_address + "/tracks/set_track/";
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/json"));
    http_manager->post(request, send_data.toUtf8());
}

void MainWindow::onSendTrack(QNetworkReply *reply) {
    if(reply->error()){
        qDebug() << "sending error";
    }
    else {
        QString result = reply->readAll();
        //qDebug() << "On send track: " << result;
        if (result == "error") {
        }
        else {
            qDebug() << "points sending: " << send_track_points.count();
            send_track_points.remove(0, ltp_count);
            ltp_count = 0;
        }
    }
    disconnect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSendTrack(QNetworkReply*)));
    reply->deleteLater();
}

void MainWindow::onTrackingButton() {
    if (ui->trackingButton->isChecked()) {
        startTracking();
    }
    else {
        endTracking();
    }
}

// получение данных треков
void MainWindow::getHuntersTracks() {
    connect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onGetHuntersTracks(QNetworkReply*)));
    QString url = "http://" + ip_address + "/tracks/get_track/?token=" + token + "&id=" + QString::number(0); //hunter_id
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    http_manager->get(request);
}

void MainWindow::onGetHuntersTracks(QNetworkReply *reply) {
    QString result;
    if(reply->error()){
    }
    else {
        QString result = reply->readAll();
        //qDebug() << "On get tracks: " << result;
        if (result == "error404") {
        }
        else {
            Track new_track;
            TrackPoint new_trackpoint;
            QJsonArray tracks_array;
            QJsonArray trackpoint_array;
            QJsonArray date_array;

            //qDebug() << "Tracks: " << result;
            QJsonDocument document = QJsonDocument::fromJson(result.toUtf8());
            if (document.isObject()) {
                hunters_tracks.clear();
                QJsonObject root = document.object();
                tracks_array = root["tracks"].toArray();
                for (int i = 0; i < tracks_array.count(); i++) {
                    new_track.hunter_id = tracks_array[i].toObject()["hunter_id"].toInt();
                    new_track.name = tracks_array[i].toObject()["n"].toString();
                    trackpoint_array = tracks_array[i].toObject()["g"].toArray();
                    for (int j = 0; j < trackpoint_array.count(); j++) {
                        new_trackpoint.x = trackpoint_array[j].toObject()["x"].toDouble();
                        new_trackpoint.y = trackpoint_array[j].toObject()["y"].toDouble();
                        date_array = trackpoint_array[j].toObject()["t"].toArray();
                        new_trackpoint.year = date_array[0].toInt();
                        new_trackpoint.month = date_array[1].toInt();
                        new_trackpoint.day = date_array[2].toInt();
                        new_trackpoint.hour = date_array[3].toInt();
                        new_trackpoint.minute = date_array[4].toInt();
                        new_track.track_points.push_back(new_trackpoint);
                    }
                    hunters_tracks.push_back(new_track);
                    new_track.track_points.clear();
                }
                updateMapTracks();
            }
        }
        //if (hunters_tracks.count() > 0)
            //qDebug() << "On get tracks: " << hunters_tracks[0].track_points[1].day;
    }
    disconnect(http_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onGetHuntersTracks(QNetworkReply*)));
    reply->deleteLater();
}

void MainWindow::updateMapTracks() {
    mapCore->clearMapS();
    for (int j = 0; j < hunters_tracks.count(); j++) {
        mapCore->addPolylineS();
        qDebug() << "Track #" << j << " Number points: " << hunters_tracks[0].track_points.count();
        for (int i = 0; i < hunters_tracks[j].track_points.count(); i++) {
            mapCore->addPointS(QGeoCoordinate(hunters_tracks[j].track_points[i].x, hunters_tracks[j].track_points[i].y));
        }
    }
}

void MainWindow::sendChatMessage() {
    QString send_data;

    send_data += "{\"tk\":\"";
    send_data += token;
    send_data += "\",\"id\":";
    send_data += QString::number(hunter_id);
    send_data += ",\"text\":\"";
    send_data += ui->messageEdit->text();
    send_data += "\"}";

    connect(http_manager2, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSendChatMessage(QNetworkReply*)));
    QString url = "http://" + ip_address + "/chat/send_message/";
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("application/json"));
    http_manager2->post(request, send_data.toUtf8());
    qDebug() << "message was send: " << send_data;
}

void MainWindow::onSendChatMessage(QNetworkReply *reply) {
    if(reply->error()){
        qDebug() << "message sending error";
    }
    else {
        QString result = reply->readAll();
        if (result == "error") {
        }
        else {
            emit messageSuccesfulSend();
        }
    }
    disconnect(http_manager2, SIGNAL(finished(QNetworkReply*)), this, SLOT(onSendChatMessage(QNetworkReply*)));
    reply->deleteLater();
}

void MainWindow::getChatMessages() {
    connect(http_manager2, SIGNAL(finished(QNetworkReply*)), this, SLOT(onGetChatMessages(QNetworkReply*)));
    QString url = "http://" + ip_address + "/chat/get_message/?token=" + token;
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    http_manager2->get(request);
}

void MainWindow::onGetChatMessages(QNetworkReply *reply) {
    QString result;
    if(reply->error()){
    }
    else {
        QString result = reply->readAll();
        //qDebug() << "On get tracks: " << result;
        if (result == "error") {
        }
        else {
            //qDebug() << "Messages get: " << result;
            ChatMessage new_message;
            QJsonArray messages_array;

            qDebug() << "Messages: " << result;
            QJsonDocument document = QJsonDocument::fromJson(result.toUtf8());
            if (document.isObject()) {
                messages.clear();
                QJsonObject root = document.object();
                messages_array = root["messages"].toArray();
                for (int i = 0; i < messages_array.count(); i++) {
                    new_message.name = messages_array[i].toObject()["name"].toString();
                    new_message.text = messages_array[i].toObject()["text"].toString();
                    messages.push_back(new_message);
                }
            }
            emit messagesSuccesfulGet();
        }
    }
    disconnect(http_manager2, SIGNAL(finished(QNetworkReply*)), this, SLOT(onGetChatMessages(QNetworkReply*)));
    reply->deleteLater();
}

void MainWindow::updateChatInterface() {
    //messagesWidget
    QWidget *delete_widget;
    int num_elems = ui->messagesWidget->layout()->count();
    for(int i = 0; i < num_elems; i++){
        delete_widget = qobject_cast<QWidget*>(ui->messagesWidget->layout()->itemAt(0)->widget());
        delete_widget->hide();
        delete delete_widget;
    }

    QLabel *new_label;

    for (int i = 0; i < messages.count(); i++) {
        new_label = new QLabel(messages[i].text);
        new_label->setStyleSheet("background-color: #fff; border-radius: 8px; padding: 5px;");
        ui->messagesWidget->layout()->addWidget(new_label);
    }
}
