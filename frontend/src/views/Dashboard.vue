<template>
  <div class="dashboard">

    <!-- ANIMATED BACKGROUND GRADIENT -->
    <div class="bg-orb orb-1" />
    <div class="bg-orb orb-2" />
    <div class="bg-orb orb-3" />

    <v-container fluid class="pa-6" style="max-width: 1400px; margin: 0 auto;">

      <!-- HERO ROW  -->
      <v-row class="mb-4" align="stretch">

        <!-- HERO CARD: Big Temperature + Status -->
        <v-col cols="12" md="5">
          <div class="glass-card hero-card h-100">
            <div class="hero-top">
              <div>
                <div class="hero-temp" :style="{ color: tempColor }">
                  {{ displayTemp }}<span class="hero-unit">°{{ tempUnit }}</span>
                </div>
                <div class="hero-label">Temperature</div>
              </div>
              <div class="hero-icon-wrap">
                <v-icon :icon="conditionIcon" size="72" :color="tempColor" class="hero-icon" />
              </div>
            </div>

            <div class="hero-meta">
              <span class="meta-pill">
                <v-icon icon="mdi:mdi-clock-outline" size="14" class="mr-1" />
                {{ currentTime }}
              </span>
              <span class="meta-pill" :class="online ? 'pill-online' : 'pill-offline'">
                <v-icon :icon="online ? 'mdi:mdi-wifi' : 'mdi:mdi-wifi-off'" size="14" class="mr-1" />
                {{ online ? 'Live' : 'Offline' }}
              </span>
              <span class="meta-pill">
                <v-icon icon="mdi:mdi-thermometer-water" size="14" class="mr-1" />
                Feels like {{ displayHeatIndex }}°{{ tempUnit }}
              </span>
            </div>

            <!-- UNIT TOGGLE -->
            <div class="unit-toggle mt-4">
              <button
                class="toggle-btn"
                :class="{ active: tempUnit === 'C' }"
                @click="setUnit('C')"
              >°C</button>
              <button
                class="toggle-btn"
                :class="{ active: tempUnit === 'F' }"
                @click="setUnit('F')"
              >°F</button>
            </div>
          </div>
        </v-col>

        <!-- MINI STAT CARDS GRID -->
        <v-col cols="12" md="7">
          <v-row class="h-100" style="gap:0">

            <v-col cols="6" class="pb-2 pr-2">
              <div class="glass-card stat-card h-100">
                <div class="stat-icon-row">
                  <v-icon icon="mdi:mdi-water-percent" size="22" color="#579ac2" />
                  <span class="stat-label">Humidity</span>
                </div>
                <div class="stat-value" style="color:#579ac2">{{ fmt(payload?.humidity, '%') }}</div>
                <div class="mini-bar-track">
                  <div class="mini-bar" :style="{ width: barW(payload?.humidity, 0, 100), background: '#579ac2' }" />
                </div>
              </div>
            </v-col>

            <v-col cols="6" class="pb-2 pl-2">
              <div class="glass-card stat-card h-100">
                <div class="stat-icon-row">
                  <v-icon icon="mdi:mdi-gauge" size="22" color="#ffd86e" />
                  <span class="stat-label">Pressure</span>
                </div>
                <div class="stat-value" style="color:#ffd86e">{{ fmt(payload?.pressure, ' hPa') }}</div>
                <div class="mini-bar-track">
                  <div class="mini-bar" :style="{ width: barW(payload?.pressure, 970, 1050), background: '#ffd86e' }" />
                </div>
              </div>
            </v-col>

            <v-col cols="6" class="pt-2 pr-2">
              <div class="glass-card stat-card h-100">
                <div class="stat-icon-row">
                  <v-icon icon="mdi:mdi-waves-arrow-up" size="22" color="#66b85f" />
                  <span class="stat-label">Altitude</span>
                </div>
                <div class="stat-value" style="color:#66b85f">{{ fmt(payload?.altitude, ' m') }}</div>
                <div class="mini-bar-track">
                  <div class="mini-bar" :style="{ width: barW(payload?.altitude, 0, 500), background: '#66b85f' }" />
                </div>
              </div>
            </v-col>

            <v-col cols="6" class="pt-2 pl-2">
              <div class="glass-card stat-card h-100">
                <div class="stat-icon-row">
                  <v-icon icon="mdi:mdi-sprout" size="22" color="#8760b5" />
                  <span class="stat-label">Soil Moisture</span>
                </div>
                <div class="stat-value" style="color:#8760b5">{{ fmt(payload?.soil, '%') }}</div>
                <div class="mini-bar-track">
                  <div class="mini-bar" :style="{ width: barW(payload?.soil, 0, 100), background: soilColor }" />
                </div>
              </div>
            </v-col>

          </v-row>
        </v-col>
      </v-row>

      <!-- CHARTS ROW -->
      <v-row class="mb-4">
        <v-col cols="12" md="8">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Temperature &amp; Heat Index</span>
              <span class="chart-badge">Live</span>
            </div>
            <div id="chart-temp" style="height:220px" />
          </div>
        </v-col>

        <v-col cols="12" md="4">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Humidity</span>
              <span class="chart-badge">Live</span>
            </div>
            <div id="chart-hum" style="height:220px" />
          </div>
        </v-col>
      </v-row>

      <!-- BOTTOM ROW: Soil + Console  -->
      <v-row>

        <!-- SOIL + PRESSURE STRIP -->
        <v-col cols="12" md="4">
          <div class="glass-card console-card">
            <div class="chart-header mb-3">
              <span class="chart-title">Device Info</span>
            </div>
            <div class="info-row" v-for="item in deviceInfo" :key="item.label">
              <span class="info-label">
                <v-icon :icon="item.icon" size="15" class="mr-1" />{{ item.label }}
              </span>
              <span class="info-value" :style="{ color: item.color || 'var(--txt)' }">{{ item.value }}</span>
            </div>
          </div>
        </v-col>

        <!-- LIVE MQTT CONSOLE -->
        <v-col cols="12" md="8">
          <div class="glass-card console-card">
            <div class="chart-header mb-2">
              <span class="chart-title">MQTT Console</span>
              <span class="chart-badge" :class="online ? '' : 'badge-off'">
                {{ consoleLog.length >= 100 ? '100+' : consoleLog.length }} msgs
              </span>
            </div>
            <div class="console-scroll" ref="consoleEl">
              <div
                v-for="(entry, i) in consoleLog"
                :key="i"
                class="console-line"
              >
                <span class="console-time">{{ entry.time }}</span>
                <span class="console-json">{{ JSON.stringify(entry.data) }}</span>
              </div>
              <div v-if="consoleLog.length === 0" class="console-empty">
                Waiting for data…
              </div>
            </div>
          </div>
        </v-col>

      </v-row>
    </v-container>
  </div>
</template>


<script setup>
import { ref, computed, watch, onMounted, onBeforeUnmount, nextTick } from 'vue'
import { useMqttStore } from '@/store/mqttStore'
import { useAppStore }  from '@/store/appStore'
import { storeToRefs }  from 'pinia'
import Highcharts       from 'highcharts'
import Exporting        from 'highcharts/modules/exporting'
Exporting(Highcharts)

// STORES 
const Mqtt     = useMqttStore()
const AppStore = useAppStore()
const { payload, payloadTopic } = storeToRefs(Mqtt)
const { consoleLog, tempUnit, themeMode }  = storeToRefs(AppStore)

// STATE 
// Tracks when the most recent payload arrived.
const lastMessageTime = ref(null)
const now = ref(Date.now())
const tempChart = ref(null)
const humChart  = ref(null)
const points    = ref(30)
const shift     = ref(false)
const currentTime = ref('')
const consoleEl   = ref(null)
let clockTimer    = null

// Computed values
// Treats the station as offline after a short gap in messages.
const online = computed(() => {
  if (!lastMessageTime.value) return false
  return (now.value - lastMessageTime.value) < 4000
})

// Converts the headline temperature into the selected unit.
const displayTemp = computed(() => {
  const v = payload.value?.temperature
  if (v == null) return '--'
  return tempUnit.value === 'F'
    ? AppStore.toFahrenheit(v).toFixed(1)
    : v.toFixed(1)
})

// Converts the heat index readout into the selected unit.
const displayHeatIndex = computed(() => {
  const v = payload.value?.heatindex
  if (v == null) return '--'
  return tempUnit.value === 'F'
    ? AppStore.toFahrenheit(v).toFixed(1)
    : v.toFixed(1)
})

// Picks the accent color for the main temperature card.
const tempColor = computed(() => {
  const t = payload.value?.temperature
  if (t == null) return 'var(--txt)'
  if (t < 18)   return '#579ac2'
  if (t < 26)   return '#66b85f'
  if (t < 32)   return '#ffd86e'
  return '#ff8a65'
})

// Colors the soil bar based on how wet the reading is.
const soilColor = computed(() => {
  const s = payload.value?.soil
  if (s == null) return '#8760b5'
  if (s < 30)   return '#ff8a65'
  if (s <= 70)  return '#66b85f'
  return '#579ac2'
})

// Chooses the hero icon from the latest conditions.
const conditionIcon = computed(() => {
  const t = payload.value?.temperature
  const h = payload.value?.humidity
  if (t == null) return 'mdi:mdi-weather-cloudy'
  if (t >= 32)  return 'mdi:mdi-weather-sunny'
  if (h > 80)   return 'mdi:mdi-weather-rainy'
  if (t < 18)   return 'mdi:mdi-snowflake'
  return 'mdi:mdi-weather-partly-cloudy'
})

// Builds the small device summary panel from the latest payload.
const deviceInfo = computed(() => [
  {
    label: 'Station ID',
    icon: 'mdi:mdi-identifier',
    value: payload.value?.id ?? '--',
  },
  {
    label: 'Last Timestamp',
    icon: 'mdi:mdi-clock-check',
    value: payload.value?.timestamp
      ? new Date(payload.value.timestamp * 1000).toLocaleTimeString()
      : '--',
  },
  {
    label: 'Topic',
    icon: 'mdi:mdi-antenna',
    value: payloadTopic.value || '--',
  },
  {
    label: 'Connection',
    icon: online.value ? 'mdi:mdi-wifi' : 'mdi:mdi-wifi-off',
    value: online.value ? 'Connected' : 'Disconnected',
    color: online.value ? '#66b85f' : '#ff8a65',
  },
  {
    label: 'Temp Unit',
    icon: 'mdi:mdi-thermometer',
    value: `°${tempUnit.value}`,
  },
])

// Helpers
// Formats stat values with one decimal place.
const fmt = (val, unit = '') => {
  if (val == null || isNaN(val)) return '--'
  return `${parseFloat(val).toFixed(1)}${unit}`
}

// Maps a sensor value into a percentage width for the mini bars.
const barW = (val, min, max) => {
  if (val == null) return '0%'
  const pct = Math.min(100, Math.max(0, ((val - min) / (max - min)) * 100))
  return `${pct}%`
}

// Updates the shared temperature unit.
const setUnit = (u) => { tempUnit.value = u }

// Keeps chart text and borders in sync with the active theme.
const chartColors = () => ({
  muted: 'var(--txt-muted)',
  line: 'var(--glass-border)',
  soft: 'var(--soft-border)',
  legend: 'var(--txt-muted)',
  tooltipBg: 'rgba(10,20,40,0.85)',
  tooltipBorder: 'var(--glass-border)',
  tooltipText: '#ffffff',
})

// Chart theme
// Shared Highcharts defaults for the live dashboard graphs.
const HC_THEME = {
  chart: {
    backgroundColor: 'transparent',
    style: { fontFamily: "'DM Mono', monospace" },
    animation: { duration: 300 },
  },
  title:   { text: '' },
  credits: { enabled: false },
  exporting: { enabled: false },
  xAxis: {
    type: 'datetime',
    labels: { style: { color: chartColors().muted, fontSize: '10px' } },
    lineColor:     chartColors().line,
    tickColor:     chartColors().line,
    gridLineColor: chartColors().soft,
    gridLineWidth: 1,
  },
  yAxis: {
    labels: { style: { color: chartColors().muted, fontSize: '10px' } },
    title:         { text: '' },
    gridLineColor: chartColors().soft,
  },
  tooltip: {
    backgroundColor: chartColors().tooltipBg,
    borderColor:     chartColors().tooltipBorder,
    style:           { color: chartColors().tooltipText },
    shared: true,
  },
  legend: {
    itemStyle: { color: chartColors().legend, fontSize: '11px' },
  },
}

// Create charts
// Creates the live temperature and humidity charts.
const createCharts = () => {
  tempChart.value = Highcharts.chart('chart-temp', {
    ...HC_THEME,
    series: [
      {
        name: 'Temperature',
        type: 'areaspline',
        data: [],
        color: '#ff8a65',
        fillColor: { linearGradient: { x1:0,y1:0,x2:0,y2:1 },
                     stops: [[0,'rgba(255,138,101,0.3)'],[1,'rgba(255,138,101,0)']] },
        lineWidth: 2,
        marker: { enabled: false },
      },
      {
        name: 'Heat Index',
        type: 'spline',
        data: [],
        color: '#ffd86e',
        lineWidth: 2,
        dashStyle: 'ShortDash',
        marker: { enabled: false },
      },
    ],
  })

  humChart.value = Highcharts.chart('chart-hum', {
    ...HC_THEME,
    yAxis: {
      ...HC_THEME.yAxis,
      min: 0, max: 100,
      labels: { ...HC_THEME.yAxis.labels, format: '{value}%' },
    },
    series: [
      {
        name: 'Humidity',
        type: 'areaspline',
        data: [],
        color: '#579ac2',
        fillColor: { linearGradient: { x1:0,y1:0,x2:0,y2:1 },
                     stops: [[0,'rgba(126,207,255,0.35)'],[1,'rgba(126,207,255,0)']] },
        lineWidth: 2,
        marker: { enabled: false },
      },
    ],
  })
}

// Push to charts
// Appends the newest reading to the live chart series.
const pushPoint = (data) => {
  if (!tempChart.value || !humChart.value) return
  const doShift = points.value <= 0
  if (points.value > 0) points.value--

  const t  = Date.now()
  const tv = tempUnit.value === 'F' ? AppStore.toFahrenheit(data.temperature) : data.temperature
  const hi = tempUnit.value === 'F' ? AppStore.toFahrenheit(data.heatindex)   : data.heatindex

  tempChart.value.series[0].addPoint({ x: t, y: parseFloat(tv.toFixed(2)) }, true, doShift)
  tempChart.value.series[1].addPoint({ x: t, y: parseFloat(hi.toFixed(2)) }, true, doShift)
  humChart.value.series[0].addPoint({ x: t, y: parseFloat(data.humidity.toFixed(2)) }, true, doShift)
}

// Lifecycle
// Bootstraps the clock, charts, and MQTT connection.
onMounted(() => {
  
  Highcharts.setOptions({ time: { useUTC: false } })
  AppStore.consoleLog = []
  createCharts()

  // clock ticker
  const tick = () => {
    currentTime.value = new Date().toLocaleTimeString()
    now.value = Date.now()
  }
  tick()
  clockTimer = setInterval(tick, 1000)

  // MQTT
  Mqtt.connect()
  setTimeout(() => Mqtt.subscribe('620172829'), 3000)
})

// Cleans up subscriptions and timers when leaving the page.
onBeforeUnmount(() => {
  Mqtt.unsubscribeAll()
  clearInterval(clockTimer)
})

// Watchers
// Refreshes the dashboard when a new payload arrives.
watch(payload, (data) => {
  if (!data) return
  lastMessageTime.value = Date.now()
  // push to console log
  AppStore.pushLog(data)

  // scroll console to top (newest first)
  nextTick(() => {
    if (consoleEl.value) consoleEl.value.scrollTop = 0
  })

  // push to charts
  pushPoint(data)
})

// Unit change handling
// Clears live series so the next points use the new unit.
watch(tempUnit, () => {
  // clear both series and refill from console log
  if (!tempChart.value) return
  tempChart.value.series[0].setData([])
  tempChart.value.series[1].setData([])
  humChart.value.series[0].setData([])
  points.value = 30
  shift.value = false
})

// Reapplies chart colors when the app theme changes.
watch(themeMode, () => {
  if (tempChart.value) {
    tempChart.value.update({ ...HC_THEME }, true, true)
  }
  if (humChart.value) {
    humChart.value.update({
      ...HC_THEME,
      yAxis: {
        ...HC_THEME.yAxis,
        min: 0,
        max: 100,
        labels: { ...HC_THEME.yAxis.labels, format: '{value}%' },
      },
    }, true, true)
  }
})
</script>


<style src="@/assets/Dashboard.css" scoped />
