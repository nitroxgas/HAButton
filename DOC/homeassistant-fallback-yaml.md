# Fallback YAML (sem discovery)

Se o discovery em `homeassistant/event/.../config` não aparecer (ACL), use:

```yaml
mqtt:
  - event:
      name: "habutton"
      unique_id: "habutton_983dae4191c0_event"
      state_topic: "habutton/983dae4191c0/event"
      device_class: button
      event_types:
        - press_a
        - press_b
        - press_c
        - long_a
        - long_b
        - long_c
        - press_ab
        - press_ac
        - press_bc
        - press_abc
        - long_ab
        - long_ac
        - long_bc
        - long_abc
      device:
        identifiers:
          - "habutton_983dae4191c0"
        name: "habutton"
        manufacturer: "HAButton"
        model: "ESP32-C3 Super Mini"
```

Ajuste o MAC. Depois: verificar YAML + reiniciar HA.

Libere ACL `homeassistant/#` para voltar ao discovery automático.
