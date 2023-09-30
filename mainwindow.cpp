#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Initialisation de l'interface graphique
    ui->setupUi(this);

    // Instanciation de la socket
    tcpSocket = new QTcpSocket(this);

    // Attachement d'un slot qui sera appelé à chaque fois que des données arrivent (mode asynchrone)
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(gerer_donnees()));

    // Idem pour les erreurs
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(afficher_erreur(QAbstractSocket::SocketError)));

    // Instanciation du timer
    pTimer = new QTimer();

    // Instanciation de la carte
    pCartePlan = new QImage();
    pCartePlan->load(":/carte_la_rochelle_plan.png");
    ui->label_carte->setPixmap(QPixmap::fromImage(*pCartePlan));

    pCarteSatellite = new QImage();
    pCarteSatellite->load(":/carte_la_rochelle_satellite.png");

    pDessin = new QImage();
    pDessin->load(":/fond_dessin.png");
    ui->label_dessin->setPixmap(QPixmap::fromImage(*pDessin));

    pCourbeFreq = new QImage();
    pCourbeFreq->load(":/fond_courbe_freq.png");
    ui->label_courbe_cardiaque->setPixmap(QPixmap::fromImage(*pCourbeFreq));

    pCourbeFreq->fill(Qt::transparent);
    timestamp = 0;
    longitude = 0.0;
    latitude = 0.0;
    lastlat= 0.0;
    lastlong = 0.0;
    px = 0.0;
    py = 0.0;
    lastpx = 0.0;
    lastpy = 0.0;
    lat_rad = 0.0;
    long_rad = 0.0;
    lastlat_rad = 0.0;
    lastlong_rad = 0.0;
    calcul_distance = 0.0;
    distance = 0.0;
    lastdistance = 0.0;
    last_timestamp = 0;
    compteur = 0;

    // Association du "tick" du timer à l'appel d'une méthode SLOT faire_qqchose()
    connect(pTimer, SIGNAL(timeout()), this, SLOT(mettre_a_jour_ihm()));

}

MainWindow::~MainWindow()
{
    // Destruction du timer
    delete pTimer;

    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Destruction de l'interface graphique
    delete ui; 

    // Arrêt du timer
    pTimer->stop();

    // Destruction de la carte
    delete pCartePlan;

    // Destruction de la carte
    delete pCarteSatellite;

    // Destruction de la carte
    delete pDessin;

    // Destruction de la carte
    delete pCourbeFreq;
}

void MainWindow::on_connexionButton_clicked()
{
    // Récupération des paramètres
    QString adresse_ip = ui->lineEdit_ip->text();
    unsigned short port_tcp = ui->lineEdit_port->text().toInt();

    // Connexion au serveur
    tcpSocket->connectToHost(adresse_ip, port_tcp);
    // Lancement du timer avec un tick toutes les 1000 ms
    pTimer->start(1000);
}

void MainWindow::on_deconnexionButton_clicked()
{
    // Déconnexion du serveur
    tcpSocket->close();
}

void MainWindow::on_envoiButton_clicked()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
}
double degToRad(double degrees) {
    return degrees * M_PI / 180.0;
}
QString calculateChecksum(const QString &nmeaFrame) {
    // Recherche de l'index du caractère '$'
    int startIndex = nmeaFrame.indexOf('$');
    if (startIndex == -1) {
        return QString(); // Pas de caractère '$' trouvé
    }

    // Recherche de l'index du caractère '*'
    int endIndex = nmeaFrame.indexOf('*', startIndex);
    if (endIndex == -1) {
        return QString(); // Pas de caractère '*' trouvé
    }

    // Extraction de la sous-chaîne entre '$' et '*' (exclusivement)
    QString subString = nmeaFrame.mid(startIndex + 1, endIndex - startIndex - 1);

    // Calcul du checksum en effectuant un XOR sur les caractères hexadécimaux
    char checksum = 0;
    for (int i = 0; i < subString.length(); i++) {
        checksum ^= subString.at(i).toLatin1();
    }

    // Formatage du checksum en hexadécimal avec deux chiffres
    return QString("%1").arg(checksum, 2, 16, QLatin1Char('0')).toLower();
}

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    QStringList liste = trame.split(",");
    QString horaire = liste[1];
    QString lat = liste[2];
    QString N_or_S = liste[3];
    QString lon = liste[4];
    QString W_or_E = liste[5];
    QString postype = liste[6];
    QString nb_satellite = liste[7];
    QString precision_horizontale = liste[8];
    QString altitude = liste[9];
    QString unite_altitude = liste[10];
    QString hauteur_geo = liste[11];
    QString unite_hauteur = liste[12];
    QString tps_last_maj = liste[13];
    QString freq_cardiaque_checksum = liste[14];

    // Calcul durée
    int heure = horaire.mid(0,2).toInt();
    int minutes = horaire.mid(2,2).toInt();
    int sec = horaire.mid(4,2).toInt();
    int premier_relevé = 28957;
    timestamp = (heure*3600)+(minutes*60)+sec;
    QString timestampQString = QString("%1").arg(timestamp-premier_relevé);
    ui->lineEdit_heure->setText(timestampQString);
    // Altitude
    ui->lineEdit_altitude->setText(altitude);

    // Latitude

    double degres_lat = lat.mid(0,2).toDouble();
    double minutes_lat = lat.mid(2,7).toDouble();
    if( N_or_S == "S"){
        latitude = (degres_lat + (minutes_lat / 60.0))*(-1.0);
    }else if(N_or_S == "N"){
        latitude = degres_lat + (minutes_lat / 60.0);

    }else{
        latitude =(degres_lat + (minutes_lat / 60.0));
    }
    QString latitude_string = QString("%1").arg(latitude);
    ui->lineEdit_lat->setText(latitude_string);

    // Longitude
    double degres_long = lon.mid(0,3).toDouble();
    double minutes_long = lon.mid(3,7).toDouble();
    if( W_or_E == "W"){
        longitude = (degres_long + (minutes_long / 60))*(-1);
    }else if(W_or_E == "E"){
        longitude = degres_long + (minutes_long / 60);

    }else{
        longitude =(degres_long + (minutes_long / 60));
    }
    QString longitude_string = QString("%1").arg(longitude);
    ui->lineEdit_long->setText(longitude_string);

    // Fréquence cardiaque
    freq = freq_cardiaque_checksum.mid(1,3).toInt();
    QString freq_string = QString("%1").arg(freq);
    ui->lineEdit_frequence_bpm->setText(freq_string);

    //Fc max

    int age = ui->spinBox_age->value();
    float fcmax = (207- ( 0.7*age));
    QString fcmax_string = QString("%1").arg(fcmax);
    ui->lineEdit_fcmax_bpm->setText(fcmax_string);

    // Changement de Carte
    if(ui->checkBox_carte->isChecked()){
        ui->label_carte->setPixmap(QPixmap::fromImage(*pCarteSatellite));

    }else{
        ui->label_carte->setPixmap(QPixmap::fromImage(*pCartePlan));
    }

    // Position projetée
    const double lat_hg = 46.173311;
    const double long_hg = -1.195703;
    const double lat_bd = 46.135451;
    const double long_bd = -1.136125;
    const double largeur_carte = 694.0;
    const double hauteur_carte = 638.0;

    px = largeur_carte*( (longitude - long_hg ) / (long_bd - long_hg) );
    py = hauteur_carte*( 1.0 - (latitude - lat_bd) / (lat_hg - lat_bd) );

    QPainter p(pDessin);
    if((lastpx != 0.0 && lastpy != 0.0)){
        p.setPen(QPen(Qt::red,2));
        p.drawLine(lastpx,lastpy, px, py);
        p.end();
        ui->label_dessin->setPixmap(QPixmap::fromImage(*pDessin));
    }else{
    }

    //Intensité
    int intensite = (freq / fcmax)*100;
    ui->progressBar->setValue(intensite);

    //Distance
    lat_rad = degToRad(latitude);
    long_rad = degToRad(longitude);
    if(long_rad && lat_rad && lastlat_rad && lastlong_rad !=0.0){
        calcul_distance = 6378.0 * acos((sin(lastlat_rad) * sin(lat_rad)) + (cos(lastlat_rad) * cos(lat_rad) * cos(lastlong_rad - long_rad)));
    distance = lastdistance + calcul_distance;
        QString distance_string = QString("%1").arg(distance);
    ui->lineEdit_distance->setText(distance_string);
    }else{
    }

    //Calories dépensées
    double poids = ui->spinBox_poids->value();
    double calories = distance * poids *1.036;
    QString calories_string = QString("%1").arg(calories);
    ui->lineEdit_2->setText(calories_string);

    //Vitesse
    double diff_tps = timestamp - last_timestamp;
    double vitesse = calcul_distance / (diff_tps /3600.0 );
    QString vitesse_string = QString("%1").arg(vitesse);
    ui->lineEdit_vitesse->setText(vitesse_string);

    // Courbes variables
    QPainter painter(pCourbeFreq);
    // Courbe fréquence
    painter.setPen(QPen(Qt::green, 1));
    painter.drawLine(compteur, 95, compteur,200 - freq);
    ui->label_HR->setStyleSheet("QLabel { color: green }");
    //Courbe altitude
    painter.setPen(QPen(Qt::red, 1));
    painter.drawLine(compteur, 200, compteur,140 - altitude.toDouble() * 2);
    ui->label_altitude->setStyleSheet("QLabel { color: red }");
    painter.end();

    compteur += 1;
    if (compteur >= ui->label_courbe_cardiaque->width()) {
    pCourbeFreq->fill(Qt::transparent);
    compteur = 0;
    }

    ui->label_courbe_cardiaque->setPixmap(QPixmap::fromImage(*pCourbeFreq));

    ui->lineEdit_reponse->setText(QString(reponse));
    qDebug() << compteur;
    qDebug() << QString(reponse);

    // Vérification de la validitée des données

    // Calcul du checksum

    QString checksum_recu = freq_cardiaque_checksum.mid(5,2);
    if(checksum_recu != calculateChecksum(trame)){
    QString invalid = "Données Non Valides";
                      ui->lineEdit_2->setText(invalid);
    ui->lineEdit_altitude->setText(invalid);
    ui->lineEdit_distance->setText(invalid);
    ui->lineEdit_fcmax_bpm->setText(invalid);
    ui->lineEdit_frequence_bpm->setText(invalid);
    ui->lineEdit_heure->setText(invalid);
    ui->lineEdit_lat->setText(invalid);
    ui->lineEdit_long->setText(invalid);
    ui->lineEdit_vitesse->setText(invalid);
    }else{

    }
}


void MainWindow::mettre_a_jour_ihm()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);
    lastpx = px ;
    lastpy = py ;
    lastlat_rad = lat_rad;
    lastlong_rad = long_rad;
    lastdistance = distance;
    last_timestamp = timestamp;
}

void MainWindow::afficher_erreur(QAbstractSocket::SocketError socketError)
{
    switch (socketError)
    {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Hôte introuvable"));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Connexion refusée"));
            break;
        default:
            QMessageBox::information(this, tr("Client TCP"),
                                     tr("Erreur : %1.")
                                     .arg(tcpSocket->errorString()));
    }
}


