#pragma once

#include "wifi_setup.h"
#include "buttons.h"

// Conecta ao broker, publica discovery (se necessario) e o acionamento.
// Retorna true se publicou com sucesso (ou nao havia botao para publicar).
bool mqttPublishButtonEvent(const AppConfig& cfg, const ButtonState& buttons);
