#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <bitset>




namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void on_connexionButton_clicked();

    void on_deconnexionButton_clicked();

    void on_envoiButton_clicked();

    void gerer_donnees();

    void afficher_erreur(QAbstractSocket::SocketError);

    void mettre_a_jour_ihm();

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    QTimer *pTimer;
    QImage *pCartePlan;
    QImage *pCarteSatellite;
    QImage *pCourbeFreq;
    QImage *pDessin;
    double longitude;
    double latitude;
    double lastlat;
    double lastlong;
    int freq;
    double px;
    double py;
    double lastpx;
    double lastpy;
    double lat_rad;
    double long_rad;
    double lastlat_rad;
    double lastlong_rad;
    double calcul_distance;
    double distance;
    double lastdistance;
    int timestamp;
    int last_timestamp;
    int compteur;
};

#endif // MAINWINDOW_H
