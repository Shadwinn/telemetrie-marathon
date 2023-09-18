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
    pCarte = new QImage();
    pCarte->load("carte_la_rochelle_plan.png");
    ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));
    //C:\\Users\\shadw\\OneDrive\\Bureau\\tp_dev_marathon\\carte_la_rochelle_plan.png

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
    distance = 0.0;
    lastdistance = 0.0;

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
    delete pCarte;


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
    QString frequence_cardiaque = liste[14];

    // Calcul durée
    int heure = horaire.mid(0,2).toInt();
    int minutes = horaire.mid(2,2).toInt();
    int sec = horaire.mid(4,2).toInt();
    int premier_relevé = 28957;
    int timestamp = (heure*3600)+(minutes*60)+sec;
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
    int freq = frequence_cardiaque.mid(1,3).toInt();
    QString freq_string = QString("%1").arg(freq);
    ui->lineEdit_frequence_bpm->setText(freq_string);

    //Fc max

    int age = ui->spinBox->value();
    float fcmax = (207- ( 0.7*age));
    QString fcmax_string = QString("%1").arg(fcmax);
    ui->lineEdit_fcmax_bpm->setText(fcmax_string);

    // Position projetée
    const double lat_hg = 46.173311;
    const double long_hg = -1.195703;
    const double lat_bd = 46.135451;
    const double long_bd = -1.136125;
    const double largeur_carte = 694.0;
    const double hauteur_carte = 638.0;

    px = largeur_carte*( (longitude - long_hg ) / (long_bd - long_hg) );
    py = hauteur_carte*( 1.0 - (latitude - lat_bd) / (lat_hg - lat_bd) );

    QPainter p(pCarte);
    if((lastpx != 0.0 && lastpy != 0.0)){
        p.setPen(Qt::red);
        p.drawLine(lastpx,lastpy, px, py);
        p.end();
        ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));
    }else{
    }

    //Intensité
    int intensite = (freq / fcmax)*100;
    ui->progressBar->setValue(intensite);

    //Distance
    lat_rad = degToRad(latitude);
    long_rad = degToRad(longitude);
    if(long_rad && lat_rad && lastlat_rad && lastlong_rad !=0.0){
        double calcul_distance = 6378.0 * acos((sin(lastlat_rad) * sin(lat_rad)) + (cos(lastlat_rad) * cos(lat_rad) * cos(lastlong_rad - long_rad)));
    distance = lastdistance + calcul_distance;
    QString distance_string = QString("%1").arg(distance);
    qDebug() << latitude;
    ui->lineEdit_distance->setText(distance_string);
    }else{
    }



    ui->lineEdit_reponse->setText(QString(reponse));
    qDebug() << QString(reponse);
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


