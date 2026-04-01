<template>
  <VToolbar flat density="compact" color="transparent" height="85">
    <VContainer fluid>
      <VRow align="center" justify="center">
        <VCol cols="2" align="left">
          <VIcon icon="mdi:mdi-weather-partly-cloudy" color="#579ac2" size="28" />
        </VCol>

        <VCol cols="8" align="center">
          <div class="nav-group">
            <RouterLink class="route" :to="{ name: 'Dashboard' }">
              <VBtn
                class="nav-btn"
                density="compact"
                :class="route.name === 'Dashboard' ? 'nav-active' : ''"
                variant="text"
                prepend-icon="mdi:mdi-view-dashboard"
              >
                Dashboard
              </VBtn>
            </RouterLink>

            <RouterLink class="route" :to="{ name: 'Analysis' }">
              <VBtn
                class="nav-btn"
                density="compact"
                :class="route.name === 'Analysis' ? 'nav-active' : ''"
                variant="text"
                prepend-icon="mdi:mdi-chart-line"
              >
                Analysis
              </VBtn>
            </RouterLink>
          </div>
        </VCol>

        <VCol cols="2" align="right">
          <button class="theme-toggle" @click="toggleTheme" :aria-label="`Switch to ${nextThemeLabel} mode`">
            <VIcon :icon="themeIcon" size="20" />
          </button>
        </VCol>
      </VRow>
    </VContainer>
  </VToolbar>
</template>

<script setup>
import '@/assets/NavBar.css'
import { computed } from 'vue'
import { useRoute } from 'vue-router'
import { useAppStore } from '@/store/appStore'

const route = useRoute()
const AppStore = useAppStore()

// Updates the button label to match the theme that will be switched to.
const nextThemeLabel = computed(() => AppStore.themeMode === 'dark' ? 'light' : 'dark')
// Swaps the icon so the toggle reflects the current theme.
const themeIcon = computed(() => AppStore.themeMode === 'dark' ? 'mdi:mdi-moon-waxing-crescent' : 'mdi:mdi-white-balance-sunny')
// Flips the shared app theme.
const toggleTheme = () => AppStore.toggleTheme()
</script>
