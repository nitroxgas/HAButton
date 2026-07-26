#pragma once

// Inicializa Serial de forma segura para alimentacao a bateria (sem host USB).
void serialBootBegin();

// flush seguro — nunca bloqueia sem CDC host.
void serialBootFlush();
