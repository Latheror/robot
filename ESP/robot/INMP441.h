#ifndef INMP441_H
#define INMP441_H

#include <Arduino.h>
#include "driver/i2s.h"
#include "settings.h"
#include <functional>

/**
 * @class INMP441
 * @brief Driver pour le microphone I2S INMP441, avec détection de double-clap.
 *
 * Cette classe gère :
 * - La configuration I2S du microphone INMP441.
 * - La lecture et le calcul du volume RMS.
 * - L’indication d’activité vocale via une LED.
 * - La détection d’un double clap et l’appel d’un callback associé.
 */
class INMP441 {
public:
    /**
     * @brief Taux d’échantillonnage par défaut (doit correspondre à celui du haut-parleur).
     */
    static constexpr int DEFAULT_SAMPLE_RATE = 24000;

    /**
     * @brief Constructeur du microphone INMP441.
     * @param sampleRate Taux d’échantillonnage souhaité (par défaut : 24000 Hz).
     */
    explicit INMP441(int sampleRate = DEFAULT_SAMPLE_RATE);

    /**
     * @brief Initialise le microphone et configure l’interface I2S.
     * @return true si l’initialisation est réussie, false sinon.
     */
    bool begin();

    /**
     * @brief Met à jour les échantillons audio, calcule le volume et détecte les claps.
     *
     * Cette méthode doit être appelée régulièrement (toutes les ~20 ms)
     * pour garantir une détection fluide et réactive.
     */
    void update();

    /**
     * @brief Lit un échantillon brut directement depuis l’I2S.
     * @return La valeur de l’échantillon sur 24 bits aligné à droite.
     */
    int32_t readSample();

    /**
     * @brief Retourne le volume courant normalisé.
     * @return Volume RMS (entre 0.0 et 1.0).
     */
    float getVolume() const;

    /**
     * @brief Indique si le volume courant dépasse le seuil d’activité vocale.
     * @return true si le volume est au-dessus du seuil, false sinon.
     */
    bool isVolumeAboveThreshold() const;

    /**
     * @brief Définit la fonction callback à appeler lorsqu’un double clap est détecté.
     * @param callback Fonction sans paramètre appelée lors d’un double clap.
     */
    void setClapCallback(std::function<void()> callback);

private:
    // --- Configuration ---
    const int _sampleRate;                ///< Taux d’échantillonnage configuré.
    const i2s_port_t _i2sPort = I2S_NUM_1; ///< Port I2S utilisé (par défaut : 1).

    // --- Données audio ---
    float _currentVolume = 0.0f;          ///< Volume RMS courant.

    // --- Timings ---
    unsigned long _lastUpdate = 0;        ///< Timestamp du dernier calcul de volume.
    unsigned long _lastDebugPrint = 0;    ///< Timestamp du dernier affichage debug.
    unsigned long _lastClapTime = 0;      ///< Dernier instant où un clap a été détecté.

    // --- Constantes ---
    static constexpr unsigned long UPDATE_INTERVAL = 20;   ///< Intervalle de mise à jour (ms).
    static constexpr unsigned long DEBUG_INTERVAL  = 1000; ///< Intervalle d’affichage debug (ms).
    static constexpr double MAX_24BIT = 8388607.0;         ///< Valeur maximale 24 bits (2^23 - 1).
    static constexpr double CLAP_THRESHOLD = 0.6;          ///< Seuil de volume (0.0 - 1.0) pour détecter un clap.
    static constexpr unsigned long CLAP_DEBOUNCE = 300;    ///< Délai anti-rebond entre deux claps (ms).

    // --- Événements ---
    std::function<void()> _clapCallback;  ///< Callback appelé lors d’un double clap.

    // --- Méthodes internes ---
    /**
     * @brief Configure le bus I2S pour la communication avec le microphone.
     * @return true si la configuration est réussie, false sinon.
     */
    bool configureI2S() const;

    /**
     * @brief Lit les échantillons du microphone et calcule le volume RMS.
     */
    void readSamplesAndComputeVolume();

    /**
     * @brief Met à jour la LED d’activité en fonction du volume courant.
     */
    void updateActivityLed();

    /**
     * @brief Détecte un double clap à partir du volume RMS.
     *
     * Utilise un système de seuil, de délai anti-rebond et de fenêtre temporelle
     * pour éviter les fausses détections.
     */
    void detectDoubleClap();
};

#endif // INMP441_H
