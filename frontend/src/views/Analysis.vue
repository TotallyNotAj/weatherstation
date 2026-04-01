<template>
  <div class="analysis">
    <div class="bg-orb orb-1" />
    <div class="bg-orb orb-2" />
    <div class="bg-orb orb-3" />

    <v-container fluid class="pa-6" style="max-width: 1400px; margin: 0 auto;">
      <v-row class="mb-4" align="stretch">
        <v-col cols="12" md="4">
          <div class="glass-card h-100">
            <div class="section-label mb-4">Date Range</div>
            <v-text-field
              v-model="start"
              label="Start Date"
              type="date"
              density="compact"
              variant="outlined"
              color="#579ac2"
              class="mb-3 an-field"
              hide-details
            />
            <v-text-field
              v-model="end"
              label="End Date"
              type="date"
              density="compact"
              variant="outlined"
              color="#579ac2"
              class="mb-4 an-field"
              hide-details
            />

            <div class="analysis-actions">
              <button class="analyze-btn" :class="{ loading: isLoading }" @click="runAnalysis">
                <v-icon icon="mdi:mdi-chart-line" size="16" class="mr-2" />
                {{ isLoading ? 'Analyzing...' : 'Analyze' }}
              </button>

              <div class="unit-toggle">
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
          </div>
        </v-col>

        <v-col cols="12" md="8">
          <v-row style="gap:0">
            <v-col cols="6" sm="4" class="pb-2 pr-2" v-for="metric in mmarMetrics" :key="metric.key">
              <div class="glass-card mmar-card h-100">
                <div class="mmar-header">
                  <v-icon :icon="metric.icon" size="16" :color="metric.color" />
                  <span class="mmar-title" :style="{ color: metric.color }">{{ metric.label }}</span>
                </div>
                <div class="mmar-avg" :style="{ color: metric.color }">
                  {{ mmar[metric.key]?.avg ?? '--' }}
                  <span class="mmar-unit">{{ metricUnit(metric) }}</span>
                </div>
                <div class="mmar-stats">
                  <div class="mmar-stat">
                    <span class="mmar-stat-label">MIN</span>
                    <span class="mmar-stat-val">{{ mmar[metric.key]?.min ?? '--' }}</span>
                  </div>
                  <div class="mmar-stat">
                    <span class="mmar-stat-label">MAX</span>
                    <span class="mmar-stat-val">{{ mmar[metric.key]?.max ?? '--' }}</span>
                  </div>
                  <div class="mmar-stat">
                    <span class="mmar-stat-label">RNG</span>
                    <span class="mmar-stat-val">{{ mmar[metric.key]?.range ?? '--' }}</span>
                  </div>
                </div>
              </div>
            </v-col>
          </v-row>
        </v-col>
      </v-row>

      <v-row class="mb-4">
        <v-col cols="12">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Temperature &amp; Heat Index Over Time</span>
            </div>
            <div id="an-chart-temp" style="height:260px" />
          </div>
        </v-col>
      </v-row>

      <v-row class="mb-4">
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Humidity Over Time</span>
            </div>
            <div id="an-chart-hum" style="height:220px" />
          </div>
        </v-col>
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Pressure Over Time</span>
            </div>
            <div id="an-chart-press" style="height:220px" />
          </div>
        </v-col>
      </v-row>

      <v-row class="mb-4">
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Altitude Over Time</span>
            </div>
            <div id="an-chart-alt" style="height:220px" />
          </div>
        </v-col>
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Soil Moisture Over Time</span>
            </div>
            <div id="an-chart-soil" style="height:220px" />
          </div>
        </v-col>
      </v-row>

      <v-row class="mb-4">
        <v-col cols="12">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Frequency Distribution</span>
            </div>
            <div id="an-chart-freq" style="height:260px" />
          </div>
        </v-col>
      </v-row>

      <v-row>
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Temperature vs Heat Index</span>
            </div>
            <div id="an-scatter-1" style="height:260px" />
          </div>
        </v-col>
        <v-col cols="12" md="6">
          <div class="glass-card chart-card">
            <div class="chart-header">
              <span class="chart-title">Humidity vs Heat Index</span>
            </div>
            <div id="an-scatter-2" style="height:260px" />
          </div>
        </v-col>
      </v-row>
    </v-container>
  </div>
</template>

<script setup>
import { ref, reactive, computed, watch, onMounted, onBeforeUnmount } from 'vue'
import { storeToRefs } from 'pinia'
import { useAppStore } from '@/store/appStore'
import { useMqttStore } from '@/store/mqttStore'
import '@/assets/Analysis.css'
import Highcharts from 'highcharts'
import more from 'highcharts/highcharts-more'
import Exporting from 'highcharts/modules/exporting'

Exporting(Highcharts)
more(Highcharts)

const AppStore = useAppStore()
const Mqtt = useMqttStore()
const { tempUnit, themeMode } = storeToRefs(AppStore)

// Holds the selected analysis range from the date inputs.
const start = ref('')
const end = ref('')
const isLoading = ref(false)

// Stores the formatted values shown in the MMAR cards.
const mmar = reactive({
  temperature: null,
  humidity: null,
  heatindex: null,
  pressure: null,
  altitude: null,
  soil: null,
})

// Keeps the raw rows so charts can be redrawn without another fetch.
const analysisRows = ref([])
const analysisMMAR = reactive({
  temperature: null,
  humidity: null,
  heatindex: null,
  pressure: null,
  altitude: null,
  soil: null,
})

const mmarMetrics = [
  { key: 'temperature', label: 'Temperature', unit: 'temp', icon: 'mdi:mdi-thermometer', color: '#ff8a65' },
  { key: 'humidity', label: 'Humidity', unit: '%', icon: 'mdi:mdi-water-percent', color: '#579ac2' },
  { key: 'heatindex', label: 'Heat Index', unit: 'temp', icon: 'mdi:mdi-thermometer-water', color: '#ffd86e' },
  { key: 'pressure', label: 'Pressure', unit: 'hPa', icon: 'mdi:mdi-gauge', color: '#66b85f' },
  { key: 'altitude', label: 'Altitude', unit: 'm', icon: 'mdi:mdi-waves-arrow-up', color: '#8760b5' },
  { key: 'soil', label: 'Soil', unit: '%', icon: 'mdi:mdi-sprout', color: '#f9a875' },
]

const charts = {}

const tempUnitLabel = computed(() => `°${tempUnit.value}`)

// Updates the shared temperature unit used across pages.
const setUnit = (unit) => {
  tempUnit.value = unit
}

// Safely converts backend values into usable numbers.
const toNum = (value) => {
  const num = Number(value)
  return Number.isFinite(num) ? num : null
}

// Converts temperature-based values into the selected unit.
const toDisplayTemp = (value) => {
  const num = toNum(value)
  if (num == null) return null
  return tempUnit.value === 'F' ? AppStore.toFahrenheit(num) : num
}

// Formats MMAR values while converting temperature metrics when needed.
const formatMetricValue = (key, value) => {
  const num = toNum(value)
  if (num == null) return null
  const display = key === 'temperature' || key === 'heatindex' ? toDisplayTemp(num) : num
  return display == null ? null : display.toFixed(1)
}

// Swaps card units between the shared temperature label and fixed units.
const metricUnit = (metric) => metric.unit === 'temp' ? tempUnitLabel.value : metric.unit

// Keeps chart text and borders in sync with the active theme.
const chartColors = () => ({
  muted: 'var(--txt-muted)',
  line: 'var(--glass-border)',
  soft: 'var(--soft-border)',
  legend: 'var(--txt-muted)',
  tooltipBg: 'rgba(10,20,40,0.9)',
  tooltipBorder: 'var(--glass-border)',
  tooltipText: '#ffffff',
  title: 'var(--txt-muted)',
})

// Shared Highcharts defaults for the analysis charts.
const HC = (extras = {}) => ({
  chart: {
    backgroundColor: 'transparent',
    style: { fontFamily: "'DM Mono', monospace" },
    zoomType: 'x',
    ...extras.chart,
  },
  title: { text: '' },
  credits: { enabled: false },
  exporting: { enabled: false },
  xAxis: {
    labels: { style: { color: chartColors().muted, fontSize: '10px' } },
    lineColor: chartColors().line,
    tickColor: chartColors().line,
    gridLineColor: chartColors().soft,
    gridLineWidth: 1,
    ...extras.xAxis,
  },
  yAxis: {
    labels: { style: { color: chartColors().muted, fontSize: '10px' } },
    title: { text: '' },
    gridLineColor: chartColors().soft,
    ...extras.yAxis,
  },
  tooltip: {
    backgroundColor: chartColors().tooltipBg,
    borderColor: chartColors().tooltipBorder,
    style: { color: chartColors().tooltipText, fontSize: '11px' },
    shared: true,
    ...extras.tooltip,
  },
  legend: {
    itemStyle: { color: chartColors().legend, fontSize: '11px' },
    ...extras.legend,
  },
  ...extras.rest,
})

// Rebuilds the MMAR card values from the cached backend response.
const renderMMAR = () => {
  for (const key of Object.keys(mmar)) {
    const data = analysisMMAR[key]
    if (!data) continue
    mmar[key] = {
      min: formatMetricValue(key, data.min),
      max: formatMetricValue(key, data.max),
      avg: formatMetricValue(key, data.avg),
      range: formatMetricValue(key, data.range),
    }
  }
}

// Rebuilds all chart series from the cached analysis rows.
const renderCharts = () => {
  if (!charts.temp || !charts.hum || !charts.press || !charts.alt || !charts.soil || !charts.scatter1 || !charts.scatter2) return

  const tempArr = []
  const hiArr = []
  const humArr = []
  const pressArr = []
  const altArr = []
  const soilArr = []

  analysisRows.value.forEach((row) => {
    const timestamp = toNum(row.timestamp)
    const temperature = toDisplayTemp(row.temperature)
    const heatindex = toDisplayTemp(row.heatindex)
    const humidity = toNum(row.humidity)
    const pressure = toNum(row.pressure)
    const altitude = toNum(row.altitude)
    const soil = toNum(row.soil)

    if (timestamp == null) return

    const x = timestamp * 1000
    if (temperature != null) tempArr.push([x, parseFloat(temperature.toFixed(2))])
    if (heatindex != null) hiArr.push([x, parseFloat(heatindex.toFixed(2))])
    if (humidity != null) humArr.push([x, parseFloat(humidity.toFixed(2))])
    if (pressure != null) pressArr.push([x, parseFloat(pressure.toFixed(2))])
    if (altitude != null) altArr.push([x, parseFloat(altitude.toFixed(2))])
    if (soil != null) soilArr.push([x, parseFloat(soil.toFixed(2))])
  })

  charts.temp.series[0].setData(tempArr)
  charts.temp.series[1].setData(hiArr)
  charts.hum.series[0].setData(humArr)
  charts.press.series[0].setData(pressArr)
  charts.alt.series[0].setData(altArr)
  charts.soil.series[0].setData(soilArr)

  charts.scatter1.series[0].setData(
    analysisRows.value
      .map((row) => [toDisplayTemp(row.temperature), toDisplayTemp(row.heatindex)])
      .filter(([temperature, heatindex]) => temperature != null && heatindex != null)
      .map(([temperature, heatindex]) => [parseFloat(temperature.toFixed(2)), parseFloat(heatindex.toFixed(2))])
  )

  charts.scatter2.series[0].setData(
    analysisRows.value
      .map((row) => [toNum(row.humidity), toDisplayTemp(row.heatindex)])
      .filter(([humidity, heatindex]) => humidity != null && heatindex != null)
      .map(([humidity, heatindex]) => [parseFloat(humidity.toFixed(2)), parseFloat(heatindex.toFixed(2))])
  )

  charts.temp.yAxis[0].update({
    labels: {
      style: { color: chartColors().muted, fontSize: '10px' },
      formatter() {
        return `${this.value}${tempUnitLabel.value}`
      },
    },
  }, false)

  charts.temp.update({
    tooltip: {
      formatter() {
        const date = Highcharts.dateFormat('%A, %b %e, %H:%M', this.x)
        const points = this.points.map((point) => `<span style="color:${point.color}">\u25cf</span> ${point.series.name}: <b>${point.y.toFixed(2)}${tempUnitLabel.value}</b>`)
        return `${date}<br/>${points.join('<br/>')}`
      },
    },
  }, false)

  charts.scatter1.xAxis[0].setTitle({ text: `Temperature (${tempUnitLabel.value})` }, false)
  charts.scatter1.yAxis[0].setTitle({ text: `Heat Index (${tempUnitLabel.value})` }, false)
  charts.scatter2.yAxis[0].setTitle({ text: `Heat Index (${tempUnitLabel.value})` }, false)

  charts.scatter1.update({
    tooltip: {
      formatter() {
        return `Temp: ${this.x.toFixed(2)}${tempUnitLabel.value}<br/>HI: ${this.y.toFixed(2)}${tempUnitLabel.value}`
      },
    },
  }, false)

  charts.scatter2.update({
    tooltip: {
      formatter() {
        return `Humidity: ${this.x.toFixed(2)}%<br/>HI: ${this.y.toFixed(2)}${tempUnitLabel.value}`
      },
    },
  }, false)

  charts.temp.redraw()
  charts.scatter1.redraw()
  charts.scatter2.redraw()
}

// Creates the analysis charts once when the view mounts.
const createCharts = () => {
  Highcharts.setOptions({ time: { useUTC: false } })

  charts.temp = Highcharts.chart('an-chart-temp', {
    ...HC({ xAxis: { type: 'datetime' } }),
    series: [
      {
        name: 'Temperature',
        type: 'areaspline',
        data: [],
        color: '#ff8a65',
        fillColor: { linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 }, stops: [[0, 'rgba(255,138,101,0.25)'], [1, 'rgba(255,138,101,0)']] },
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

  charts.hum = Highcharts.chart('an-chart-hum', {
    ...HC({
      xAxis: { type: 'datetime' },
      yAxis: {
        labels: {
          style: { color: chartColors().muted, fontSize: '10px' },
          format: '{value}%',
        },
        title: { text: '' },
        gridLineColor: chartColors().soft,
      },
    }),
    series: [
      {
        name: 'Humidity',
        type: 'areaspline',
        data: [],
        color: '#579ac2',
        fillColor: { linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 }, stops: [[0, 'rgba(126,207,255,0.25)'], [1, 'rgba(126,207,255,0)']] },
        lineWidth: 2,
        marker: { enabled: false },
      },
    ],
  })

  charts.press = Highcharts.chart('an-chart-press', {
    ...HC({ xAxis: { type: 'datetime' } }),
    series: [
      {
        name: 'Pressure',
        type: 'areaspline',
        data: [],
        color: '#66b85f',
        fillColor: { linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 }, stops: [[0, 'rgba(168,230,163,0.25)'], [1, 'rgba(168,230,163,0)']] },
        lineWidth: 2,
        marker: { enabled: false },
      },
    ],
  })

  charts.alt = Highcharts.chart('an-chart-alt', {
    ...HC({ xAxis: { type: 'datetime' } }),
    series: [
      {
        name: 'Altitude',
        type: 'spline',
        data: [],
        color: '#8760b5',
        lineWidth: 2,
        marker: { enabled: false },
      },
    ],
  })

  charts.soil = Highcharts.chart('an-chart-soil', {
    ...HC({ xAxis: { type: 'datetime' } }),
    series: [
      {
        name: 'Soil Moisture',
        type: 'areaspline',
        data: [],
        color: '#f9a875',
        fillColor: { linearGradient: { x1: 0, y1: 0, x2: 0, y2: 1 }, stops: [[0, 'rgba(249,168,117,0.25)'], [1, 'rgba(249,168,117,0)']] },
        lineWidth: 2,
        marker: { enabled: false },
      },
    ],
  })

  charts.freq = Highcharts.chart('an-chart-freq', {
    ...HC({ xAxis: { title: { text: 'Range', style: { color: chartColors().title } } } }),
    series: [
      { name: 'Temperature', type: 'column', data: [], color: '#ff8a65', borderRadius: 4 },
      { name: 'Humidity', type: 'column', data: [], color: '#579ac2', borderRadius: 4 },
      { name: 'Heat Index', type: 'column', data: [], color: '#ffd86e', borderRadius: 4 },
    ],
  })

  charts.scatter1 = Highcharts.chart('an-scatter-1', {
    ...HC({
      chart: { type: 'scatter', zoomType: 'xy' },
      xAxis: {
        title: { text: `Temperature (${tempUnitLabel.value})`, style: { color: chartColors().title, fontSize: '10px' } },
        labels: { style: { color: chartColors().muted, fontSize: '10px' } },
        gridLineColor: chartColors().soft,
        gridLineWidth: 1,
      },
      yAxis: {
        title: { text: `Heat Index (${tempUnitLabel.value})`, style: { color: chartColors().title, fontSize: '10px' } },
        labels: { style: { color: chartColors().muted, fontSize: '10px' } },
        gridLineColor: chartColors().soft,
      },
      tooltip: {
        formatter() {
          return `Temp: ${this.x.toFixed(2)}${tempUnitLabel.value}<br/>HI: ${this.y.toFixed(2)}${tempUnitLabel.value}`
        },
        backgroundColor: 'rgba(10,20,40,0.9)',
        borderColor: chartColors().tooltipBorder,
        style: { color: chartColors().tooltipText, fontSize: '11px' },
      },
    }),
    plotOptions: { scatter: { marker: { radius: 3, symbol: 'circle' }, jitter: { x: 0.005 } } },
    series: [{ name: 'Temp vs HI', data: [], color: '#ff8a65', marker: { radius: 3 } }],
  })

  charts.scatter2 = Highcharts.chart('an-scatter-2', {
    ...HC({
      chart: { type: 'scatter', zoomType: 'xy' },
      xAxis: {
        title: { text: 'Humidity (%)', style: { color: chartColors().title, fontSize: '10px' } },
        labels: { style: { color: chartColors().muted, fontSize: '10px' } },
        gridLineColor: chartColors().soft,
        gridLineWidth: 1,
      },
      yAxis: {
        title: { text: `Heat Index (${tempUnitLabel.value})`, style: { color: chartColors().title, fontSize: '10px' } },
        labels: { style: { color: chartColors().muted, fontSize: '10px' } },
        gridLineColor: chartColors().soft,
      },
      tooltip: {
        formatter() {
          return `Humidity: ${this.x.toFixed(2)}%<br/>HI: ${this.y.toFixed(2)}${tempUnitLabel.value}`
        },
        backgroundColor: 'rgba(10,20,40,0.9)',
        borderColor: chartColors().tooltipBorder,
        style: { color: chartColors().tooltipText, fontSize: '11px' },
      },
    }),
    plotOptions: { scatter: { marker: { radius: 3, symbol: 'circle' }, jitter: { x: 0.005 } } },
    series: [{ name: 'Humidity vs HI', data: [], color: '#579ac2', marker: { radius: 3 } }],
  })
}

// Fetches the selected range and pushes the results into cards and charts.
const runAnalysis = async () => {
  if (!start.value || !end.value) return
  isLoading.value = true

  const startTs = new Date(`${start.value}T00:00:00`).getTime() / 1000
  const endTs = new Date(`${end.value}T23:59:59`).getTime() / 1000

  try {
    const [rawData, allMMAR, freqTemp, freqHum, freqHI] = await Promise.all([
      AppStore.getAllInRange(startTs, endTs),
      AppStore.getAllMMAR(startTs, endTs),
      AppStore.getFreqDistro('temperature', startTs, endTs),
      AppStore.getFreqDistro('humidity', startTs, endTs),
      AppStore.getFreqDistro('heatindex', startTs, endTs),
    ])

    analysisRows.value = rawData
    for (const key of Object.keys(analysisMMAR)) {
      analysisMMAR[key] = allMMAR[key]?.[0] ?? null
    }

    renderMMAR()
    renderCharts()

    charts.freq.series[0].setData(freqTemp.map((row) => ({ x: row._id === 'outliers' ? 110 : row._id, y: row.count })))
    charts.freq.series[1].setData(freqHum.map((row) => ({ x: row._id === 'outliers' ? 110 : row._id, y: row.count })))
    charts.freq.series[2].setData(freqHI.map((row) => ({ x: row._id === 'outliers' ? 110 : row._id, y: row.count })))
  } catch (error) {
    console.error('Analysis error:', error)
  } finally {
    isLoading.value = false
  }
}

// Redraws temperature-based views when the unit changes.
watch(tempUnit, () => {
  renderMMAR()
  if (analysisRows.value.length) {
    renderCharts()
  }
})

// Refreshes chart colors when the app theme changes.
watch(themeMode, () => {
  if (analysisRows.value.length) {
    renderCharts()
  }
})

// Creates charts and starts the MQTT client for the page session.
onMounted(() => {
  createCharts()
  Mqtt.connect()
  setTimeout(() => Mqtt.subscribe('620172829'), 3000)
})

// Stops MQTT subscriptions when leaving the page.
onBeforeUnmount(() => {
  Mqtt.unsubscribeAll()
})
</script>
