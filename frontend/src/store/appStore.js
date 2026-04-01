import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useAppStore = defineStore('app', () => {

    /*
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions
    */

    const apiFetch = async (url, label) => {
        const controller = new AbortController()
        const id = setTimeout(() => controller.abort(), 60000)
        try {
            const response = await fetch(url, { method: 'GET', signal: controller.signal })
            clearTimeout(id)
            if (response.ok) {
                const data = await response.json()
                if (data?.status === 'found') return data.data
                console.log(`${label} returned no data`)
            } else {
                console.log(`${label} HTTP error:`, await response.text())
            }
        } catch (err) {
            console.error(`${label} error:`, err.message)
        }
        return []
    }

    const latestReading = ref(null)
    const consoleLog = ref([])
    const tempUnit = ref('C')
    const themeMode = ref('dark')

    const toFahrenheit = (c) => +(c * 9 / 5 + 32).toFixed(2)
    const toCelsius = (f) => +((f - 32) * 5 / 9).toFixed(2)

    const convertTemp = (val) => {
        if (val == null || isNaN(val)) return null
        return tempUnit.value === 'F' ? toFahrenheit(val) : +val.toFixed(2)
    }

    const toggleUnit = () => {
        tempUnit.value = tempUnit.value === 'C' ? 'F' : 'C'
    }

    const toggleTheme = () => {
        themeMode.value = themeMode.value === 'dark' ? 'light' : 'dark'
    }

    const pushLog = (payload) => {
        const entry = {
            time: new Date().toLocaleTimeString(),
            data: payload
        }
        consoleLog.value.unshift(entry)
        if (consoleLog.value.length > 100) {
            consoleLog.value.pop()
        }
    }

    const getAllInRange = async (start, end) => {
        return await apiFetch(`/api/weather/get/${start}/${end}`, 'getAllInRange')
    }

    const getTemperatureMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/temperature/${start}/${end}`, 'getTemperatureMMAR')
    }

    const getHumidityMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/humidity/${start}/${end}`, 'getHumidityMMAR')
    }

    const getHeatIndexMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/heatindex/${start}/${end}`, 'getHeatIndexMMAR')
    }

    const getPressureMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/pressure/${start}/${end}`, 'getPressureMMAR')
    }

    const getAltitudeMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/altitude/${start}/${end}`, 'getAltitudeMMAR')
    }

    const getSoilMMAR = async (start, end) => {
        return await apiFetch(`/api/mmar/soil/${start}/${end}`, 'getSoilMMAR')
    }

    const getAllMMAR = async (start, end) => {
        const [temperature, humidity, heatindex, pressure, altitude, soil] = await Promise.all([
            getTemperatureMMAR(start, end),
            getHumidityMMAR(start, end),
            getHeatIndexMMAR(start, end),
            getPressureMMAR(start, end),
            getAltitudeMMAR(start, end),
            getSoilMMAR(start, end),
        ])
        return { temperature, humidity, heatindex, pressure, altitude, soil }
    }

    const getFreqDistro = async (variable, start, end) => {
        return await apiFetch(`/api/frequency/${variable}/${start}/${end}`, 'getFreqDistro')
    }

    return {
        latestReading,
        consoleLog,
        tempUnit,
        themeMode,
        convertTemp,
        toFahrenheit,
        toCelsius,
        toggleUnit,
        toggleTheme,
        pushLog,
        getAllInRange,
        getTemperatureMMAR,
        getHumidityMMAR,
        getHeatIndexMMAR,
        getPressureMMAR,
        getAltitudeMMAR,
        getSoilMMAR,
        getAllMMAR,
        getFreqDistro,
    }

}, { persist: true })
