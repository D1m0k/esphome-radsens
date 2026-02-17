# Grafana setup (ESPHome RadSens + InfluxDB)

1. Добавь Data Source типа InfluxDB в Grafana.
2. Импортируй дашборд из `grafana/radsens-dashboard.json`.
3. При импорте выбери свой `datasource`.
4. Проверь соответствие entity_id в переменных дашборда.

## Что обычно нужно подставить в переменных

- `sensor.radsens_dynamic_intensity`
- `sensor.radsens_static_intensity`
- `sensor.radsens_counts_per_minute`
- `sensor.radsens_counts_per_polling`
- `sensor.radsens_radsens_max_cpm`
- `sensor.radsens_radsens_max_cpp`
- `sensor.radsens_total_cpp`
- `sensor.radsens_uptime`
- `sensor.radsens_accumulated_dose_msv`

Если имя узла ESPHome другое, entity_id будут с другим префиксом — подставь их в переменные дашборда.
