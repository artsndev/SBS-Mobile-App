<template>
  <div>
    <v-app-bar :elevation="0" color="transparent">
      <template v-slot:prepend>
        <v-btn icon="mdi-weather-sunny"></v-btn>
      </template>
      <!-- <v-spacer></v-spacer> -->
      <v-app-bar-title class="text-center fs-18">Smart Blind Stick <span class="text-grey">v1.0</span></v-app-bar-title>
      <template v-slot:append>
        <v-btn icon="mdi-help-circle-outline"></v-btn>
      </template>
    </v-app-bar>
    <p class="text-center mt-n5 fs-12">by artisandev</p>
    <v-container class="mt-15">
      <v-row >
        <v-col cols="12">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="100" color="red">mdi-heart-pulse</v-icon>
            <p class="fs-28 font-weight-bold">63<span class="fs-14 text-grey ms-2">bpm</span></p>
            <p class="fs-14">Average BPM</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="80" color="red">mdi-gas-cylinder</v-icon>
            <p class="fs-28 font-weight-bold">23</p>
            <p class="fs-14">Oxygen Level</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="80" color="red">mdi-water-outline</v-icon>
            <p class="fs-28 font-weight-bold">Yes</p>
            <p class="fs-14">Water Sensor</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="80" color="red">mdi-motion-sensor</v-icon>
            <p class="fs-28 font-weight-bold">23<span class="fs-14 text-grey ms-2">cm</span></p>
            <p class="fs-14">Top Sensor</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="80" color="red">mdi-motion-sensor</v-icon>
            <p class="fs-28 font-weight-bold">252<span class="fs-14 text-grey ms-2">cm</span></p>
            <p class="fs-14">Bottom Sensor</p>
          </v-card>
        </v-col>
      </v-row>
      <div class="text-center mt-8">
        <p class="text-grey fs-12">@QuirkyQuarks Squadrons</p>
        <p class="text-grey fs-12">© 2024 All rights reserved.</p>
      </div>
    </v-container>
    <!-- <div v-if="sensorData">
      <p>Front Distance: {{ sensorData.distanceFront }} cm</p>
      <p>Side Distance: {{ sensorData.distanceSide }} cm</p>
      <p>Water Detected: {{ sensorData.waterDetected ? 'Yes' : 'No' }}</p>
      <p>BPM: {{ sensorData.bpm }}</p>
      <p>Average BPM: {{ sensorData.avgBpm }}</p>
      <p>SpO2: {{ sensorData.spo2 }}%</p>
    </div>
    <div v-else>
      <p>Loading data...</p>
    </div> -->
  </div>
</template>

<script setup>
import { BASE_URL } from '@/server';
import { ref, onMounted, onBeforeUnmount } from 'vue';

const sensorData = ref(null);
let eventSource = null;

// Initialize EventSource and handle incoming data
const setupEventSource = () => {
  eventSource = new EventSource(BASE_URL + '/sensor-data');

  eventSource.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      sensorData.value = data;
    } catch (error) {
      console.error("Error parsing sensor data:", error);
    }
  };

  eventSource.onerror = (error) => {
    console.error("EventSource error:", error);
    if (eventSource.readyState === EventSource.CLOSED) {
      console.log("Connection was closed. Reconnecting...");
      setupEventSource();  // Optional: Reconnect logic
    }
  };
};

// Close EventSource when the component is destroyed
onBeforeUnmount(() => {
  if (eventSource) {
    eventSource.close();
  }
});

// Setup EventSource when the component is mounted
onMounted(() => {
  setupEventSource();
});
</script>

<style scoped>
.fs-12{
  font-size: 12px;
}
.fs-14{
  font-size: 14px;
}
.fs-18{
  font-size: 18px;
}
.fs-28{
  font-size: 28px;
}
</style>
