<template>
  <div>
    <h1>ESP32 Sensor Data</h1>
    <div v-if="sensorData">
      <p>Front Distance: {{ sensorData.distanceFront }} cm</p>
      <p>Side Distance: {{ sensorData.distanceSide }} cm</p>
      <p>Water Detected: {{ sensorData.waterDetected ? 'Yes' : 'No' }}</p>
      <p>BPM: {{ sensorData.bpm }}</p>
      <p>Average BPM: {{ sensorData.avgBpm }}</p>
      <p>SpO2: {{ sensorData.spo2 }}%</p>
    </div>
    <div v-else>
      <p>Loading data...</p>
    </div>
  </div>
</template>

<script>
import axios from 'axios';

export default {
  data() {
    return {
      sensorData: null
    };
  },
  mounted() {
    this.fetchSensorData();
  },
  methods: {
    async fetchSensorData() {
      try {
        const response = await axios.get('http://192.168.1.99/sensor-data');
        this.sensorData = response.data;
      } catch (error) {
        console.error("Error fetching sensor data:", error);
      }
    }
  }
}
</script>
