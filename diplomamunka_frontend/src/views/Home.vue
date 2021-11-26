<template>
  <div class="home">
    <div v-if="role === 'SuperAdmin'" class="text-center p-5">
      <h1>Szia SuperAdmin!</h1>
      <h3>Kérjük válassz eljárást a felső menübárból!</h3>
    </div>
    <div v-else >
      <device-data />
    </div>
  </div>
</template>

<script>

import { mapGetters } from 'vuex'
import DeviceData from '../pages/DeviceData/DeviceData'

export default {
  name: 'Home',
  components: {
    DeviceData,
  },
  computed: {
    ...mapGetters({
      role: 'getRole',
    })
  }
}
</script>
