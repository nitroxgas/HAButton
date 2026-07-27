# Home Assistant — counters and automations

**Language:** [English](homeassistant-counters.md) · [Português](../homeassistant-contadores.md)

Two approaches:

1. **[Via UI](#via-ui-helpers--paste-automation)** — Helpers + paste YAML in the editor (no file editing).
2. **[Via YAML files](#via-yaml-files-packages)** — package in `config/packages/` + include in `configuration.yaml`.

HAButton publishes **one** MQTT Event entity. Each gesture changes the `event_type` attribute.

> Adjust `event.habutton` to the real `entity_id`  
> (**Settings → Devices → habutton**, e.g. `event.habutton_983dae4191c0_event`).

**Do not mix** the same automation created in the UI and in the package (duplicate IDs/aliases). Pick one path.

## event_types

| event_type | Meaning |
|------------|---------|
| `press_a` / `press_b` / `press_c` | Short press |
| `long_a` / `long_b` / `long_c` | Long press (~≥800 ms) |
| `press_ab` / `press_ac` / `press_bc` / `press_abc` | Short press combo |
| `long_ab` / `long_ac` / `long_bc` / `long_abc` | Long press combo |

## Via UI (Helpers + paste automation)

## Current format (paste in editor)

In recent HA versions, automation YAML uses:

| Old (do not use) | Current (paste in UI) |
|------------------|-------------------------|
| `trigger:` + `platform: state` | `triggers:` + `trigger: state` |
| `condition:` | `conditions:` |
| `action:` / `service:` | `actions:` + `action: domain.service` |
| List under `automation:` in YAML | **One** document (no `automation:` key) |

### How to paste

1. **Settings → Automations & scenes → Create automation → Create new automation**
2. Focus the editor (blank area) and press **Ctrl+V** (Mac: **Cmd+V**), **or**
3. Menu ⋮ → **Edit in YAML** → paste → **Save**

HA converts the pasted YAML to the visual editor.  
Do not edit `automations.yaml` by hand.

---

## 1) Counters (Helpers — no YAML)

For each `event_type` you want to count:

1. **Settings → Devices & services → Helpers → Create helper**
2. Type **Counter**
3. Suggested name: `HAButton press_a` (HA generates `counter.habutton_press_a`)
4. Initial `0`, step `1`

Create all 14 counters (or only the ones you need), with names that result in:

`counter.habutton_<event_type>`  
e.g. `counter.habutton_press_a`, `counter.habutton_long_abc`

---

## 2) Automation — count all events

Paste this into a **new automation** (replace the event `entity_id`):

```yaml
alias: HAButton count all events
description: Increments counter.habutton_<event_type> on each gesture
mode: queued
max: 20
triggers:
  - trigger: state
    entity_id: event.habutton
variables:
  et: "{{ trigger.to_state.attributes.event_type | default('') }}"
  counter_id: "counter.habutton_{{ et }}"
conditions:
  - condition: template
    value_template: "{{ et != '' and states(counter_id) not in ['unknown', 'unavailable', 'none'] }}"
actions:
  - action: counter.increment
    target:
      entity_id: "{{ counter_id }}"
```

Requirements: helpers `counter.habutton_*` already created with the suffix matching `event_type`.

### Variant with `choose` (explicit)

Use if you prefer fixed branches (also paste in the editor):

```yaml
alias: HAButton count (choose)
description: Counts each event_type in its counter
mode: queued
max: 20
triggers:
  - trigger: state
    entity_id: event.habutton
variables:
  et: "{{ trigger.to_state.attributes.event_type | default('') }}"
conditions:
  - condition: template
    value_template: >-
      {{ et in [
        'press_a','press_b','press_c',
        'long_a','long_b','long_c',
        'press_ab','press_ac','press_bc','press_abc',
        'long_ab','long_ac','long_bc','long_abc'
      ] }}
actions:
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_a' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_a
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_b' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_b
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_c' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_c
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_a' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_a
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_b' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_b
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_c' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_c
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_ab' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_ab
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_ac' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_ac
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_bc' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_bc
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_abc' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_press_abc
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_ab' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_ab
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_ac' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_ac
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_bc' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_bc
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_abc' }}"
        sequence:
          - action: counter.increment
            target:
              entity_id: counter.habutton_long_abc
```

---

## 3) Automation — actions per gesture

Paste into a **second** automation (independent from the counting one). Replace entities with yours:

```yaml
alias: HAButton gestures → actions
description: Actions per event_type (edit the entities)
mode: queued
max: 20
triggers:
  - trigger: state
    entity_id: event.habutton
variables:
  et: "{{ trigger.to_state.attributes.event_type | default('') }}"
conditions: []
actions:
  - choose:
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_a' }}"
        sequence:
          - action: light.toggle
            target:
              entity_id: light.sala
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_b' }}"
        sequence:
          - action: switch.toggle
            target:
              entity_id: switch.abajur
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_c' }}"
        sequence:
          - action: script.modo_cinema
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_a' }}"
        sequence:
          - action: light.turn_off
            target:
              entity_id: light.sala
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_b' }}"
        sequence:
          - action: scene.turn_on
            target:
              entity_id: scene.noite
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_c' }}"
        sequence:
          - action: lock.lock
            target:
              entity_id: lock.porta
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_ab' }}"
        sequence:
          - action: media_player.media_play_pause
            target:
              entity_id: media_player.tv
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_ac' }}"
        sequence:
          - action: climate.set_temperature
            target:
              entity_id: climate.sala
            data:
              temperature: 22
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_bc' }}"
        sequence:
          - action: cover.toggle
            target:
              entity_id: cover.persiana
      - conditions:
          - condition: template
            value_template: "{{ et == 'press_abc' }}"
        sequence:
          - action: script.cheguei_em_casa
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_ab' }}"
        sequence:
          - action: alarm_control_panel.alarm_arm_home
            target:
              entity_id: alarm_control_panel.casa
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_ac' }}"
        sequence:
          - action: vacuum.start
            target:
              entity_id: vacuum.aspirador
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_bc' }}"
        sequence:
          - action: fan.toggle
            target:
              entity_id: fan.teto
      - conditions:
          - condition: template
            value_template: "{{ et == 'long_abc' }}"
        sequence:
          - action: script.emergencia
```

### Single gesture (minimal)

```yaml
alias: HAButton press_a → toggle light
description: ""
mode: single
triggers:
  - trigger: state
    entity_id: event.habutton
conditions:
  - condition: template
    value_template: "{{ trigger.to_state.attributes.event_type == 'press_a' }}"
actions:
  - action: light.toggle
    target:
      entity_id: light.sala
```

---

## 4) Totals by hour / day / week / month (optional, still in UI)

Without editing files:

1. **Helpers → Create helper → Template**  
   - State: `{{ states('counter.habutton_press_a') | int(0) }}`  
   - State class: **Total increasing** (`total_increasing`)  
   - Unit: `events`
2. **Helpers → Create helper → Utility Meter**  
   - Source: the template sensor above  
   - Cycle: hourly / daily / weekly / monthly  

Repeat only for the gestures that matter.

---

## How to test

1. **Developer tools → States** → `event.…` → `event_type` attribute
2. Trigger a gesture → `counter.habutton_press_a` (etc.) should increment
3. In the automation → **Traces** to see trigger / choose

## Checklist (UI)

- [ ] `event.…` entity visible (MQTT discovery)
- [ ] Counters created via **Helpers** (`counter.habutton_<event_type>`)
- [ ] Counting automation pasted in editor (`triggers` / `actions` format)
- [ ] Actions automation pasted (your entities)
- [ ] (Optional) Template + Utility Meter via Helpers

---

## Via YAML files (packages)

Use when you want to version everything in files or create all 14 counters + meters at once.

### Where files live

Home Assistant configuration folder (same place as `configuration.yaml`):

| Installation | Typical path |
|--------------|--------------|
| Home Assistant OS / Supervised | `/config/` (File Editor / Samba / Studio Code Server) |
| Docker container | mounted volume, e.g. `/home/…/homeassistant/` → `/config` |
| Core (venv) | directory passed in `-c` / where `configuration.yaml` lives |

Suggested structure:

```text
config/                          # or /config on HA OS
├── configuration.yaml           # main file (edit)
├── automations.yaml             # managed by UI — do not mix here
└── packages/                    # create this folder
    └── habutton.yaml            # HAButton package (create)
```

### Step 1 — Enable packages in `configuration.yaml`

Open `configuration.yaml` and ensure the `homeassistant:` block includes `packages`.

If `homeassistant:` **already exists**:

```yaml
homeassistant:
  # ... other options you already have (name, unit_system, etc.)
  packages: !include_dir_named packages
```

If `homeassistant:` **does not exist**, add:

```yaml
homeassistant:
  packages: !include_dir_named packages
```

> Indentation: `packages:` must be **2 spaces** inside `homeassistant:`.  
> `!include_dir_named packages` loads each `*.yaml` from the `packages/` folder (the file name becomes the package name).

Also keep the default line for UI automations (do not remove):

```yaml
automation: !include automations.yaml
```

Package automations use a different key (`automation habutton:` below) and coexist with UI ones.

### Step 2 — Create the folder and file

1. Create the `packages` folder next to `configuration.yaml`.
2. Create `packages/habutton.yaml` with the content from the next section.
3. Replace `event.habutton` with the real `entity_id` everywhere.

### Step 3 — Contents of `packages/habutton.yaml`

The package file **does not** repeat the `packages:` key.  
Place domains at the root level, like in `configuration.yaml`.

**MQTT compatibility (firmware ≥ 1.4.2 / schema 7):** the package **does not** reference MQTT topics — only the event `entity_id` and `attributes.event_type`. With the rollback, the contract is again `{base}/event` + `{"event_type":"..."}` and `unique_id` `…_event`. **You do not need to change the package structure**; just verify that `entity_id` still points to the correct entity (e.g. `event.habutton` or `event.habutton_<mac>_event`). If during the broken period HA created `…_e`, update `entity_id` in the package to the restored `…_event` entity.

```yaml
# packages/habutton.yaml
# HAButton package — counters, counting automation, templates and utility_meters.
# Replace event.habutton with the real entity_id.

counter:
  habutton_press_a:
    name: "HAButton press_a"
    initial: 0
    step: 1
  habutton_press_b:
    name: "HAButton press_b"
    initial: 0
    step: 1
  habutton_press_c:
    name: "HAButton press_c"
    initial: 0
    step: 1
  habutton_long_a:
    name: "HAButton long_a"
    initial: 0
    step: 1
  habutton_long_b:
    name: "HAButton long_b"
    initial: 0
    step: 1
  habutton_long_c:
    name: "HAButton long_c"
    initial: 0
    step: 1
  habutton_press_ab:
    name: "HAButton press_ab"
    initial: 0
    step: 1
  habutton_press_ac:
    name: "HAButton press_ac"
    initial: 0
    step: 1
  habutton_press_bc:
    name: "HAButton press_bc"
    initial: 0
    step: 1
  habutton_press_abc:
    name: "HAButton press_abc"
    initial: 0
    step: 1
  habutton_long_ab:
    name: "HAButton long_ab"
    initial: 0
    step: 1
  habutton_long_ac:
    name: "HAButton long_ac"
    initial: 0
    step: 1
  habutton_long_bc:
    name: "HAButton long_bc"
    initial: 0
    step: 1
  habutton_long_abc:
    name: "HAButton long_abc"
    initial: 0
    step: 1

# Labeled key: does not conflict with automation: !include automations.yaml
automation habutton:
  - id: habutton_count_all_events
    alias: "HAButton count all events"
    description: "Increments counter.habutton_<event_type>"
    mode: queued
    max: 20
    triggers:
      - trigger: state
        entity_id: event.habutton
    variables:
      et: "{{ trigger.to_state.attributes.event_type | default('') }}"
      counter_id: "counter.habutton_{{ et }}"
    conditions:
      - condition: template
        value_template: "{{ et != '' and states(counter_id) not in ['unknown', 'unavailable', 'none'] }}"
    actions:
      - action: counter.increment
        target:
          entity_id: "{{ counter_id }}"

  - id: habutton_gestos_acoes
    alias: "HAButton gestures → actions"
    description: "Edit the target entities"
    mode: queued
    max: 20
    triggers:
      - trigger: state
        entity_id: event.habutton
    variables:
      et: "{{ trigger.to_state.attributes.event_type | default('') }}"
    actions:
      - choose:
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_a' }}"
            sequence:
              - action: light.toggle
                target:
                  entity_id: light.sala
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_b' }}"
            sequence:
              - action: switch.toggle
                target:
                  entity_id: switch.abajur
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_c' }}"
            sequence:
              - action: script.modo_cinema
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_a' }}"
            sequence:
              - action: light.turn_off
                target:
                  entity_id: light.sala
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_b' }}"
            sequence:
              - action: scene.turn_on
                target:
                  entity_id: scene.noite
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_c' }}"
            sequence:
              - action: lock.lock
                target:
                  entity_id: lock.porta
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_ab' }}"
            sequence:
              - action: media_player.media_play_pause
                target:
                  entity_id: media_player.tv
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_ac' }}"
            sequence:
              - action: climate.set_temperature
                target:
                  entity_id: climate.sala
                data:
                  temperature: 22
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_bc' }}"
            sequence:
              - action: cover.toggle
                target:
                  entity_id: cover.persiana
          - conditions:
              - condition: template
                value_template: "{{ et == 'press_abc' }}"
            sequence:
              - action: script.cheguei_em_casa
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_ab' }}"
            sequence:
              - action: alarm_control_panel.alarm_arm_home
                target:
                  entity_id: alarm_control_panel.casa
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_ac' }}"
            sequence:
              - action: vacuum.start
                target:
                  entity_id: vacuum.aspirador
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_bc' }}"
            sequence:
              - action: fan.toggle
                target:
                  entity_id: fan.teto
          - conditions:
              - condition: template
                value_template: "{{ et == 'long_abc' }}"
            sequence:
              - action: script.emergencia

template:
  - sensor:
      - name: "HAButton press_a total"
        unique_id: habutton_press_a_total
        state: "{{ states('counter.habutton_press_a') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_b total"
        unique_id: habutton_press_b_total
        state: "{{ states('counter.habutton_press_b') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_c total"
        unique_id: habutton_press_c_total
        state: "{{ states('counter.habutton_press_c') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_a total"
        unique_id: habutton_long_a_total
        state: "{{ states('counter.habutton_long_a') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_b total"
        unique_id: habutton_long_b_total
        state: "{{ states('counter.habutton_long_b') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_c total"
        unique_id: habutton_long_c_total
        state: "{{ states('counter.habutton_long_c') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_ab total"
        unique_id: habutton_press_ab_total
        state: "{{ states('counter.habutton_press_ab') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_ac total"
        unique_id: habutton_press_ac_total
        state: "{{ states('counter.habutton_press_ac') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_bc total"
        unique_id: habutton_press_bc_total
        state: "{{ states('counter.habutton_press_bc') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton press_abc total"
        unique_id: habutton_press_abc_total
        state: "{{ states('counter.habutton_press_abc') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_ab total"
        unique_id: habutton_long_ab_total
        state: "{{ states('counter.habutton_long_ab') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_ac total"
        unique_id: habutton_long_ac_total
        state: "{{ states('counter.habutton_long_ac') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_bc total"
        unique_id: habutton_long_bc_total
        state: "{{ states('counter.habutton_long_bc') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"
      - name: "HAButton long_abc total"
        unique_id: habutton_long_abc_total
        state: "{{ states('counter.habutton_long_abc') | int(0) }}"
        state_class: total_increasing
        unit_of_measurement: "events"

utility_meter:
  habutton_press_a_hourly:
    source: sensor.habutton_press_a_total
    cycle: hourly
  habutton_press_a_daily:
    source: sensor.habutton_press_a_total
    cycle: daily
  habutton_press_a_weekly:
    source: sensor.habutton_press_a_total
    cycle: weekly
  habutton_press_a_monthly:
    source: sensor.habutton_press_a_total
    cycle: monthly
  habutton_press_b_hourly:
    source: sensor.habutton_press_b_total
    cycle: hourly
  habutton_press_b_daily:
    source: sensor.habutton_press_b_total
    cycle: daily
  habutton_press_b_weekly:
    source: sensor.habutton_press_b_total
    cycle: weekly
  habutton_press_b_monthly:
    source: sensor.habutton_press_b_total
    cycle: monthly
  habutton_press_c_hourly:
    source: sensor.habutton_press_c_total
    cycle: hourly
  habutton_press_c_daily:
    source: sensor.habutton_press_c_total
    cycle: daily
  habutton_press_c_weekly:
    source: sensor.habutton_press_c_total
    cycle: weekly
  habutton_press_c_monthly:
    source: sensor.habutton_press_c_total
    cycle: monthly
  habutton_long_a_hourly:
    source: sensor.habutton_long_a_total
    cycle: hourly
  habutton_long_a_daily:
    source: sensor.habutton_long_a_total
    cycle: daily
  habutton_long_a_weekly:
    source: sensor.habutton_long_a_total
    cycle: weekly
  habutton_long_a_monthly:
    source: sensor.habutton_long_a_total
    cycle: monthly
  habutton_long_b_hourly:
    source: sensor.habutton_long_b_total
    cycle: hourly
  habutton_long_b_daily:
    source: sensor.habutton_long_b_total
    cycle: daily
  habutton_long_b_weekly:
    source: sensor.habutton_long_b_total
    cycle: weekly
  habutton_long_b_monthly:
    source: sensor.habutton_long_b_total
    cycle: monthly
  habutton_long_c_hourly:
    source: sensor.habutton_long_c_total
    cycle: hourly
  habutton_long_c_daily:
    source: sensor.habutton_long_c_total
    cycle: daily
  habutton_long_c_weekly:
    source: sensor.habutton_long_c_total
    cycle: weekly
  habutton_long_c_monthly:
    source: sensor.habutton_long_c_total
    cycle: monthly
  habutton_press_ab_hourly:
    source: sensor.habutton_press_ab_total
    cycle: hourly
  habutton_press_ab_daily:
    source: sensor.habutton_press_ab_total
    cycle: daily
  habutton_press_ab_weekly:
    source: sensor.habutton_press_ab_total
    cycle: weekly
  habutton_press_ab_monthly:
    source: sensor.habutton_press_ab_total
    cycle: monthly
  habutton_press_ac_hourly:
    source: sensor.habutton_press_ac_total
    cycle: hourly
  habutton_press_ac_daily:
    source: sensor.habutton_press_ac_total
    cycle: daily
  habutton_press_ac_weekly:
    source: sensor.habutton_press_ac_total
    cycle: weekly
  habutton_press_ac_monthly:
    source: sensor.habutton_press_ac_total
    cycle: monthly
  habutton_press_bc_hourly:
    source: sensor.habutton_press_bc_total
    cycle: hourly
  habutton_press_bc_daily:
    source: sensor.habutton_press_bc_total
    cycle: daily
  habutton_press_bc_weekly:
    source: sensor.habutton_press_bc_total
    cycle: weekly
  habutton_press_bc_monthly:
    source: sensor.habutton_press_bc_total
    cycle: monthly
  habutton_press_abc_hourly:
    source: sensor.habutton_press_abc_total
    cycle: hourly
  habutton_press_abc_daily:
    source: sensor.habutton_press_abc_total
    cycle: daily
  habutton_press_abc_weekly:
    source: sensor.habutton_press_abc_total
    cycle: weekly
  habutton_press_abc_monthly:
    source: sensor.habutton_press_abc_total
    cycle: monthly
  habutton_long_ab_hourly:
    source: sensor.habutton_long_ab_total
    cycle: hourly
  habutton_long_ab_daily:
    source: sensor.habutton_long_ab_total
    cycle: daily
  habutton_long_ab_weekly:
    source: sensor.habutton_long_ab_total
    cycle: weekly
  habutton_long_ab_monthly:
    source: sensor.habutton_long_ab_total
    cycle: monthly
  habutton_long_ac_hourly:
    source: sensor.habutton_long_ac_total
    cycle: hourly
  habutton_long_ac_daily:
    source: sensor.habutton_long_ac_total
    cycle: daily
  habutton_long_ac_weekly:
    source: sensor.habutton_long_ac_total
    cycle: weekly
  habutton_long_ac_monthly:
    source: sensor.habutton_long_ac_total
    cycle: monthly
  habutton_long_bc_hourly:
    source: sensor.habutton_long_bc_total
    cycle: hourly
  habutton_long_bc_daily:
    source: sensor.habutton_long_bc_total
    cycle: daily
  habutton_long_bc_weekly:
    source: sensor.habutton_long_bc_total
    cycle: weekly
  habutton_long_bc_monthly:
    source: sensor.habutton_long_bc_total
    cycle: monthly
  habutton_long_abc_hourly:
    source: sensor.habutton_long_abc_total
    cycle: hourly
  habutton_long_abc_daily:
    source: sensor.habutton_long_abc_total
    cycle: daily
  habutton_long_abc_weekly:
    source: sensor.habutton_long_abc_total
    cycle: weekly
  habutton_long_abc_monthly:
    source: sensor.habutton_long_abc_total
    cycle: monthly
```

You can remove from the package the `utility_meter` / `template` / action automations you do not need.

### Step 4 — Validate and apply

1. **Developer tools → YAML → Check configuration**  
   (or **Settings → System → Restart → Quick check**)
2. If OK: **Restart Home Assistant**  
   (packages / `counter` / `template` / `utility_meter` require restart; automation alone sometimes reloads, but restart is the safe path)
3. Check entities: `counter.habutton_press_a`, `sensor.habutton_press_a_total`, `sensor.habutton_press_a_daily`, etc.
4. Package automations appear in **Automations & scenes** (may be read-only in the UI if they come from labeled YAML)

### Alternative without `packages/` folder

Everything in `configuration.yaml` (less organized):

```yaml
homeassistant:
  # ...

counter:
  habutton_press_a:
    name: "HAButton press_a"
    initial: 0
    step: 1
  # ... remaining counters

automation habutton:
  - id: habutton_count_all_events
    alias: "HAButton count all events"
    mode: queued
    triggers:
      - trigger: state
        entity_id: event.habutton
    # ... rest same as package
```

Or a single included file:

```yaml
# configuration.yaml
homeassistant:
  packages:
    habutton: !include packages/habutton.yaml
```

(in this mode the content of `habutton.yaml` is the same as in Step 3.)

### Common conflicts

| Symptom | Cause / fix |
|---------|-------------|
| `packages` integration not found | `packages:` outside `homeassistant:` — indent 2 spaces |
| Entity ID already exists | Counter/helper already created in UI with same id — delete the helper or rename in YAML |
| Duplicate automation | Same `id`/`alias` in UI and package — remove one side |
| Invalid YAML | Indentation; use configuration checker before restart |

### Checklist (files)

- [ ] `packages/` folder created next to `configuration.yaml`
- [ ] `homeassistant: packages: !include_dir_named packages` in `configuration.yaml`
- [ ] `packages/habutton.yaml` with counters + `automation habutton:`
- [ ] `event.habutton` adjusted to the real `entity_id`
- [ ] Configuration checked + Home Assistant restarted
- [ ] No duplicate counters/automations in the UI
