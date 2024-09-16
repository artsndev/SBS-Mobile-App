<template>
  <div>
    <v-app-bar :elevation="0" color="transparent">
      <template v-slot:prepend>
        <v-btn icon="mdi-weather-sunny"></v-btn>
      </template>
      <v-app-bar-title class="text-center fs-18">Smart Blind Stick <span class="text-grey">v1.0</span></v-app-bar-title>
      <template v-slot:append>
        <v-btn icon="mdi-help-circle-outline"></v-btn>
      </template>
    </v-app-bar>
    <p class="text-center mt-n5 fs-14">with Pulse Sensor</p>
    <v-container class="mt-15">
      <v-row  v-if="sensorData">
        <v-col cols="12">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="100" color="red">mdi-heart-pulse</v-icon>
            <p class="fs-28 font-weight-bold">{{ sensorData.avgBpm }}<span class="fs-14 text-grey ms-2">bpm</span></p>
            <p class="fs-14">Average BPM</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="70" color="red">mdi-pulse</v-icon>
            <p class="fs-20 font-weight-bold">{{ sensorData.bpm }}<span class="fs-14 text-grey ms-2">bpm</span></p>
            <p class="fs-14">Pulse Per Minute</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="70" color="red">mdi-water-outline</v-icon>
            <p class="fs-20 font-weight-bold">{{ sensorData.waterDetected ? 'Yes' : 'No' }}</p>
            <p class="fs-14">Water Sensor</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="70" color="red">mdi-motion-sensor</v-icon>
            <p class="fs-20 font-weight-bold">{{ sensorData.distanceFront }}<span class="fs-14 text-grey ms-2">cm</span></p>
            <p class="fs-14">Top Sensor</p>
          </v-card>
        </v-col>
        <v-col cols="6">
          <v-card class="text-center py-5" rounded="xl" color="blue-grey-darken-4">
            <v-icon size="70" color="red">mdi-motion-sensor</v-icon>
            <p class="fs-20 font-weight-bold">{{ sensorData.distanceSide }}<span class="fs-14 text-grey ms-2">cm</span></p>
            <p class="fs-14">Bottom Sensor</p>
          </v-card>
        </v-col>
      </v-row>
      <div class="text-center" v-else>
        <h1>No data</h1>
      </div>
      <div class="text-center mt-16">
        <p class="text-grey fs-12">@QuirkyQuarks Squadrons</p>
        <p class="text-grey fs-12">© 2024 All rights reserved.</p>
      </div>

    </v-container>
      <!-- <p>BPM: {{ sensorData.bpm }}</p> -->
  </div>
</template>

<script setup>
import { BASE_URL } from '@/server';
import axios from 'axios';
import { ref, onMounted, onBeforeUnmount } from 'vue';

const sensorData = ref(null)
let intervalId = null
const fetchSensorData = async () => {
  try {
    const response = await axios.get(BASE_URL + '/sensor-data')
    sensorData.value = response.data
  } catch (error) {
    console.log('Error fetching data: '+ error)
  }
}

onMounted(() => {
  fetchSensorData();
  intervalId = setInterval(fetchSensorData, 5000);
});

onBeforeUnmount(() => {
  if (intervalId) {
    clearInterval(intervalId);
  }
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
.fs-20{
  font-size: 20px;
}
.fs-28{
  font-size: 28px;
}
</style>
