<template>
  <VApp :class="['app-shell', `theme-${themeMode}`]">
    <NavBar />

    <VMain class="app-main">
      <RouterView v-slot="{ Component, route }">
        <transition :name="route.meta.transition || 'fade'" mode="out-in">
          <component :is="Component" />
        </transition>
      </RouterView>
    </VMain>
  </VApp>
</template>

<script setup>
import { RouterView } from 'vue-router'
import { watch } from 'vue'
import { storeToRefs } from 'pinia'
import NavBar from '@/components/NavBar.vue'
import { useAppStore } from '@/store/appStore'

const AppStore = useAppStore()
const { themeMode } = storeToRefs(AppStore)

watch(themeMode, (mode) => {
  document.body.style.backgroundColor = mode === 'light' ? '#a9bcd6' : '#040d1a'
}, { immediate: true })
</script>

<style>
html, body {
  margin: 0;
  padding: 0;
}

body {
  background: var(--page-bg, #040d1a) !important;
  color: var(--txt, #e8f0ff);
  transition: background-color 0.3s ease, color 0.3s ease;
}

.app-shell {
  min-height: 100vh;
  background: var(--page-bg) !important;
  color: var(--txt);
  transition: background-color 0.3s ease, color 0.3s ease;
}

.app-shell.theme-dark {
  --page-bg: #040d1a;
  --txt: #e8f0ff;
  --txt-muted: rgba(200,215,255,0.45);
  --glass-bg: rgba(255,255,255,0.05);
  --glass-border: rgba(255,255,255,0.1);
  --glass-hover-border: rgba(255,255,255,0.18);
  --soft-border: rgba(255,255,255,0.08);
  --line-border: rgba(255,255,255,0.05);
  --panel-soft: rgba(255,255,255,0.06);
  --panel-strong: rgba(255,255,255,0.15);
  --nav-border: rgba(255,255,255,0.06);
}

.app-shell.theme-light {
  --page-bg: #a9bcd6;
  --txt: #1f2c3d;
  --txt-muted: rgba(31,44,61,0.62);
  --glass-bg: rgba(255,255,255,0.22);
  --glass-border: rgba(255,255,255,0.34);
  --glass-hover-border: rgba(255,255,255,0.5);
  --soft-border: rgba(31,44,61,0.12);
  --line-border: rgba(31,44,61,0.1);
  --panel-soft: rgba(255,255,255,0.24);
  --panel-strong: rgba(255,255,255,0.42);
  --nav-border: rgba(31,44,61,0.1);
}

.app-main,
.v-main {
  background: var(--page-bg) !important;
  padding: 0 !important;
  transition: background-color 0.3s ease;
}

.theme-light .bg-orb {
  opacity: 0.11;
}

.slide-enter-active,
.slide-leave-active {
  transition: opacity 0.5s, transform 0.5s;
}

.slide-enter-from,
.slide-leave-to {
  opacity: 0;
}

.slide-enter-to,
.slide-leave-from {
  opacity: 1;
  transform: translateX(10px);
}

.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.3s ease;
}

.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}

.bounce-enter-active {
  animation: bounce-in 0.5s;
}

.bounce-leave-active {
  animation: bounce-in 0.5s reverse;
}

@keyframes bounce-in {
  0% { transform: scale(0); }
  50% { transform: scale(1.25); }
  100% { transform: scale(1); }
}

.slide-fade-enter-active {
  transition: all 0.3s ease-out;
}

.slide-fade-leave-active {
  transition: all 0.8s cubic-bezier(1, 0.5, 0.8, 1);
}

.slide-fade-enter-from,
.slide-fade-leave-to {
  transform: translateX(20px);
  opacity: 0;
}
</style>
