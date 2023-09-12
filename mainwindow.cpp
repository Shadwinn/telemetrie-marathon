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
    pCarte->load(".\\carte_la_rochelle_plan.png");

    // Association du "tick" du timer à l'appel d'une méthode SLOT faire_qqchose()
    connect(pTimer, SIGNAL(timeout()), this, SLOT(mettre_a_jour_ihm()));

    // Lancement du timer avec un tick toutes les 1000 ms
    pTimer->start(1000);
}

MainWindow::~MainWindow()
{
    // Destruction de la socket
    tcpSocket->abort();
    delete tcpSocket;

    // Destruction de l'interface graphique
    delete ui;

    // Arrêt du timer
    pTimer->stop();

    // Destruction du timer
    delete pTimer;

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

void MainWindow::gerer_donnees()
{
    // Réception des données
    QByteArray reponse = tcpSocket->readAll();
    QString trame = QString(reponse);
    QStringList liste = trame.split(",");
    QString horaire = liste[1];
    QString latitude = liste[2];
    QString nord_ou_sud = liste[3];
    QString longitude = liste[4];
    QString est_ou_ouest = liste[5];
    QString postype = liste[6];
    QString nb_satellite = liste[7];
    QString precision_horizontale = liste[8];
    QString altitude = liste[9];
    QString hauteur_geo = liste[10];
    QString unite_altitude = liste[11];
    QString tps_last_maj = liste[12];
    QString id_station_ref = liste[13];
    QString checksum = liste[14];

    // Calculs
    QString heure = horaire.mid(0,2);
    QString minutes = horaire.mid(2,2);
    QString sec = horaire.mid(4,2);

    // Affichage
    ui->lineEdit_reponse->setText(QString(reponse));
    ui->lineEdit_heure->setText(heure+" h "+minutes+" min "+sec+" sec ");
    ui->label_carte->setPixmap(QPixmap::fromImage(*pCarte));
    ui->label_carte->setFixedSize(pCarte->width(), pCarte->height());

    qDebug() << QString(reponse);
}


void MainWindow::mettre_a_jour_ihm()
{
    // Préparation de la requête
    QByteArray requete;
    requete = "RETR\r\n";

    // Envoi de la requête
    tcpSocket->write(requete);

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


