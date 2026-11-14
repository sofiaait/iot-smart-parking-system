#include <Wire.h>
#include "rgb_lcd.h"
#include <Servo.h>

// === LCD ===
rgb_lcd lcd;

// === Servo ===
Servo barriere;
const int pinServo = 4;

// === Capteurs entrée/sortie ===
const int trigEntree = 8;
const int echoEntree = 9;
const int trigSortie = 2;
const int echoSortie = 3;

// === Capteurs de place ===
const int nbPlaces = 4;
const int trigPins[nbPlaces] = {5, 7, 11, A0};
const int echoPins[nbPlaces] = {6, 10, 12, A1};
int etatPlaces[nbPlaces] = {0};  // 0 = libre, 1 = occupée
int placesDispo = nbPlaces;

const int seuilDetection = 10; // cm

void setup() {
  Serial.begin(9600);

  // Servo
  barriere.attach(pinServo);
  barriere.write(0); // barrière fermée

  // Capteurs entrée/sortie
  pinMode(trigEntree, OUTPUT);
  pinMode(echoEntree, INPUT);
  pinMode(trigSortie, OUTPUT);
  pinMode(echoSortie, INPUT);

  // Capteurs des places
  for (int i = 0; i < nbPlaces; i++) {
    pinMode(trigPins[i], OUTPUT);
    pinMode(echoPins[i], INPUT);
  }

  // LCD
  Wire.begin(); // SDA = A4, SCL = A5
  lcd.begin(16, 2);
  lcd.setRGB(0, 255, 0);
  afficherPlaces();
}

void loop() {
  verifierPlaces(); // met à jour placesDispo automatiquement

  // Capteur d'entrée
  int dEntree = mesurerDistance(trigEntree, echoEntree);
  Serial.print("Entrée : "); Serial.println(dEntree);
  if (dEntree > 0 && dEntree < 10 && placesDispo > 0) {
    ouvrirBarriere();
    delay(2000); // anti-rebond
  }

  int dSortie = mesurerDistance(trigSortie, echoSortie);
Serial.print("Sortie : "); Serial.println(dSortie);

// Si une voiture est détectée à moins de 10 cm et qu'il y a au moins une voiture dans le parking
if (dSortie > 0 && dSortie < 10 && placesDispo <= nbPlaces) {
  ouvrirBarriere();
  delay(3000); // attend 3 secondes avant de refermer
}

}

// === Mesure distance ===
int mesurerDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duree = pulseIn(echo, HIGH, 20000);
  int distance = duree * 0.034 / 2;
  return (duree == 0) ? -1 : distance;
}

// === Ouvrir puis refermer barrière ===
void ouvrirBarriere() {
  barriere.write(90);
  delay(3000);
  barriere.write(0);
}

// === Vérifier état des places ===
void verifierPlaces() {
  int occupees = 0;

  for (int i = 0; i < nbPlaces; i++) {
    int d = mesurerDistance(trigPins[i], echoPins[i]);

    if (d != -1) {
      if (d < seuilDetection) {
        etatPlaces[i] = 1;
      } else {
        etatPlaces[i] = 0;
      }
    }
    
    // On compte seulement les places considérées occupées
    if (etatPlaces[i] == 1) {
      occupees++;
    }

    // Debug console
    Serial.print("Place ");
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.print(d);
    Serial.print(" cm  -> ");
    Serial.println(etatPlaces[i] == 1 ? "OCCUPEE" : "LIBRE");
  }

  int nouvellesPlaces = nbPlaces - occupees;
  if (nouvellesPlaces != placesDispo) {
    placesDispo = nouvellesPlaces;
    afficherPlaces();
  }
}

// === Affichage LCD ===
void afficherPlaces() {
  lcd.setCursor(0, 0);
  lcd.print("                "); // Efface
  lcd.setCursor(0, 1);
  lcd.print("                ");

  if (placesDispo > 0) {
    lcd.setCursor(0, 0);
    lcd.print("Dispo:");
    lcd.setCursor(0, 1);
    lcd.print(placesDispo);
    lcd.setRGB(0, 255, 0); // Vert
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Parking");
    lcd.setCursor(0, 1);
    lcd.print("COMPLET");
    lcd.setRGB(255, 0, 0); // Rouge
  }
}
